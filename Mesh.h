#ifndef MESH_H
#define MESH_H

#include <GL/glfw.h>


typedef struct {
        float x,y,z;
}Normal, *pNormal, Vector3f, *pVector3f;

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

//typedef struct {
//        unsigned int pointIndex[3];
//        Normal normal[3];
//        unsigned int uvIndex[3];
//}Triangle, *pTriangle;

//typedef struct {
//        float r,g,b,a;
//}RGBA;

typedef struct {
        unsigned int *face;
        unsigned int numFaces;
}SharedFaces, *pSharedFaces;

//typedef struct {
//        float u,v;
//}UVCoord, *pUVCoord;

typedef struct {
        GLuint id;
}Texture, *pTexture;

void MeshInit();

class Mesh {
public:
    Mesh(){vertex=NULL; face=NULL; numVertices=0; numFaces=0;}
    ~Mesh(){ delete[] vertex; delete[] face; delete[] tangent;}

    void Init(unsigned int verts, unsigned int faces);
    void Scale(float s);
    void Scale(float sx, float sy, float sz);
    void CalculateNormals();
    void CreateBuffers();
    void Draw();

    pVertex vertex;
    pFace face;
    pSharedFaces sharedFaces;
    pVector3f tangent;

    GLuint vBufferID;
    GLuint iBufferID;

    unsigned int numVertices;
    unsigned int numFaces;
private:
};

#endif
