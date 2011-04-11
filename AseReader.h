#ifndef ASEREADER_H
#define ASEREADER_H
#include <string>
#include "Mesh.h"
#include "Entity.h"

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

int LoadAse(const string &filename, StaticEntity &entity);
StaticEntity &LoadAse(const string &filename);

#endif
