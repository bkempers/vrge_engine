//
//  mesh.cpp
//  game_engine
//
//  Created by Ben Kempers on 5/5/24.
//

#include "../include/mesh.hpp"

Mesh::Mesh(){
    
};
    
//Mesh::~Mesh(){
////    glDeleteVertexArrays(1, &vao);
////    glDeleteBuffers(1, &vbo);
////    glDeleteBuffers(1, &ebo);
//};
    
void Mesh::setupMesh(){
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);  // Generate EBO

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);  // Bind EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Vertex attribute pointers remain the same
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // Free memory
    vao_size = indices.size();  // Now using the size of the indices
    std::vector<Vertex>().swap(vertices);
    std::vector<unsigned int>().swap(indices);  // Clear index data from memory
};
    
void Mesh::draw(){
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, static_cast<int>(vao_size), GL_UNSIGNED_INT, 0);  // Use glDrawElements
    glBindVertexArray(0);
};
