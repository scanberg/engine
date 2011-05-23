#ifndef ResourceManager_H
#define ResourceManager_H

#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glfw.h>

#include <string>
#include <map>

#include "Entity.h"

class ResourceManager
{
public:
    ~ResourceManager();
    GLuint loadTexture(std::string,int flags=0);
    GLuint createTextureFromImage(std::string,GLFWimage *img, int flags=0);
    MeshObject* loadMeshObject(std::string);
    void clear();
private:
    std::map<std::string, GLuint> textureMap;
    std::map<std::string, MeshObject*> meshMap;
};

#endif
