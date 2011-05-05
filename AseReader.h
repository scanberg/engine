#ifndef ASEREADER_H
#define ASEREADER_H
#include <string>
#include "Entity.h"
#include "Mesh.h"
#include "Material.h"

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

int LoadAse(const std::string &filename, StaticEntity &entity, float scale=1.0);
StaticEntity *LoadAse(const std::string &filename);

#endif
