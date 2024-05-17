//
//  drop.cpp
//  game_engine
//
//  Created by Ben Kempers on 5/17/24.
//

#include "drop.hpp"

void Drop::descend(double* h, double* p, double* b, bool* track, double* pd, glm::ivec2 dim, double scale){

  glm::ivec2 ipos;

  while(volume > minVol){

    //Initial Position
    ipos = pos;
    int ind = ipos.x*dim.y+ipos.y;

    //Add to Path
    track[ind] = true;

    glm::vec3 n = surfaceNormal(ind, h, dim, scale);

    //Effective Parameter Set
    /* Higher plant density means less erosion */
    double effD = depositionRate*std::max(0.0, 1.0-pd[ind]);

    /* Lower Friction, Lower Evaporation in Streams
    makes particles prefer established streams -> "curvy" */
    double effF = friction*(1.0-0.5*p[ind]);
    double effR = evapRate*(1.0-0.2*p[ind]);

    //Newtonian Mechanics
    glm::vec2 acc = glm::vec2(n.x, n.z)/(float)(volume*density);
    speed += dt*acc;
    pos   += dt*speed;
    speed *= (1.0-dt*effF);

    //New Position
    int nind = (int)pos.x*dim.y+(int)pos.y;

    //Out-Of-Bounds
    if(!glm::all(glm::greaterThanEqual(pos, glm::vec2(0))) ||
       !glm::all(glm::lessThan((glm::ivec2)pos, dim))){
         volume = 0.0;
         break;
       }

    //Particle is not accelerated
    if(p[nind] > 0.3 && length(acc) < 0.01)
      break;

    //Particle enters Pool
    if(b[nind] > 0.0)
      break;

    //Mass-Transfer (in MASS)
    double c_eq = fmax(0.0, glm::length(speed)*(h[ind]-h[nind]));
    double cdiff = c_eq - sediment;
    sediment += dt*effD*cdiff;
    h[ind] -= volume*dt*effD*cdiff;

    //Evaporate (Mass Conservative)
    sediment /= (1.0-dt*effR);
    volume *= (1.0-dt*effR);
  }
};

void Drop::flood(double* h, double* p, glm::ivec2 dim){

  //Current Height
  index = (int)pos.x*dim.y + (int)pos.y;
  double plane = h[index] + p[index];
  double initialplane = plane;

  //Floodset
  std::vector<int> set;
  int fail = 10;

  //Iterate
  while(volume > minVol && fail){

    set.clear();
    int size = dim.x*dim.y;
    bool tried[size];
    memset( tried, false, size*sizeof(bool) );

    int drain;
    bool drainfound = false;

    std::function<void(int)> floodfill = [&](int i){
      //Out of Bounds
      if(i < 0 || i >= size) return;

      //Position has been tried
      if(tried[i]) return;
      tried[i] = true;

      //Wall / Boundary
      if(plane < h[i] + p[i]) return;

      //Drainage Point
      if(initialplane > h[i] + p[i]){

        //No Drain yet
        if(!drainfound)
          drain = i;

        //Lower Drain
        else if( p[drain] + h[drain] < p[i] + h[i] )
          drain = i;

        drainfound = true;
        return;
      }

      set.push_back(i);
      floodfill(i+1);
      floodfill(i+dim.y+1);  //Fill Diagonals
      floodfill(i+dim.y);    //Fill Neighbors
      floodfill(i+dim.y-1);
      floodfill(i-1);
      floodfill(i-dim.y-1);
      floodfill(i-dim.y);
      floodfill(i-dim.y+1);
    };

    //Perform Flood
    floodfill(index);

    //Drainage Point
    if(drainfound){

      //Set the Drop Position and Evaporate
      pos = glm::vec2(drain/dim.y, drain%dim.y);

      //Set the New Waterlevel (Slowly)
      double drainage = 0.1;
      plane = (1.0-drainage)*initialplane + drainage*(h[drain] + p[drain]);

      //Compute the New Height
      for(auto& s: set)
        p[s] = (plane > h[s])?(plane-h[s]):0.0;

      //Remove Sediment
      sediment *= 0.1;
      break;
    }

    //Get Volume under Plane
    double tVol = 0.0;
    for(auto& s: set)
      tVol += volumeFactor*(plane - (h[s]+p[s]));

    //We can partially fill this volume
    if(tVol <= volume && initialplane < plane){

      //Raise water level to plane height
      for(auto& s: set)
        p[s] = plane - h[s];

      //Adjust Drop Volume
      volume -= tVol;
      tVol = 0.0;
    }

    //Plane was too high.
    else fail--;

    //Adjust Planes
    initialplane = (plane > initialplane)?plane:initialplane;
    plane += 0.8*(volume-tVol)/(double)set.size()/volumeFactor;
  }

  //Couldn't place the volume (for some reason)- so ignore this drop.
  if(fail == 0)
    volume = 0.0;
}

glm::vec3 Drop::surfaceNormal(int index, double* h, glm::ivec2 dim, double scale){

  glm::vec3 n = glm::vec3(0.0);

  //Two large triangles adjacent to the plane (+Y -> +X) (-Y -> -X)
  for(int i = 1; i <= 1; i++){
    n += (1.0f/(float)i*i)*glm::cross(glm::vec3(0.0, scale*(h[index+i]-h[index]), i), glm::vec3( i, scale*(h[index+i*dim.y]-h[index]), 0.0));
    n += (1.0f/(float)i*i)*glm::cross(glm::vec3(0.0, scale*(h[index-i]-h[index]),-i), glm::vec3(-i, scale*(h[index-i*dim.y]-h[index]), 0.0));
    n += (1.0f/(float)i*i)*glm::cross(glm::vec3( i, scale*(h[index+i*dim.y]-h[index]), 0.0), glm::vec3(0.0, scale*(h[index-i]-h[index]),-i));
    n += (1.0f/(float)i*i)*glm::cross(glm::vec3(-i, scale*(h[index-i*dim.y]-h[index]), 0.0), glm::vec3(0.0, scale*(h[index+i]-h[index]), i));
  }

  return glm::normalize(n);
}