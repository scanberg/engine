#ifndef MESH_H
#define MESH_H

#include <GL/glfw.h>
#include "Vector3f.h"


typedef struct {
        float u,v;
}Vector2f, *pVector2f;

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
    void createBuffers();
    void draw();

    Vector3f center;

    pVertex vertex;
    pFace face;
    pVector3f tangent;

    GLuint vBufferID;
    GLuint iBufferID;

    unsigned int numVertices;
    unsigned int numFaces;
private:
};

#endif
