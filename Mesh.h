#ifndef MESH_H
#define MESH_H


#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <math.h>
#include "Math.h"

typedef struct {
        float u,v;
        float nx,ny,nz;
        float x,y,z;
}Vertex, *pVertex;

typedef struct {
        unsigned int point[3];
}Face, *pFace;

void MeshInit();

class Mesh {
public:
    Mesh(){vertex=NULL; face=NULL; numVertices=0; numFaces=0;}
    ~Mesh(){ delete[] vertex; delete[] face; delete[] tangent;}

    void init(unsigned int verts, unsigned int faces);
    void scale(float s);
    void scale(float sx, float sy, float sz);
    void calculateNormals();
    void calculateTangents();
    void createBuffers();
    void draw();

    //Vector3f center;
    glm::vec3 center;

    pVertex vertex;
    pFace face;
    //pVector3f tangent;
    glm::vec3 *tangent;

    unsigned int vBufferID;
    unsigned int iBufferID;

    unsigned int numVertices;
    unsigned int numFaces;
private:
};

#endif
