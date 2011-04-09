#ifndef ASEREADER_H
#define ASEREADER_H
#include <string>
#include "Mesh.h"

using std::string;

typedef struct {
        float x,y,z;
        float nx,ny,nz;
        float u,v;
        unsigned int val;
        unsigned int sg;
}TVertex;

typedef struct {
        unsigned int point[3];
        unsigned int sg;
}TFace;

int LoadAse(string filename, Mesh &mesh);

#endif
