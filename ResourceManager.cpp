#include "ResourceManager.h"
#include "AseReader.h"

ResourceManager::~ResourceManager()
{
    std::map<std::string, MeshObject*>::iterator mit;
    mit=meshMap.begin();
    while(mit!=meshMap.end())
    {
        delete mit->second;
        mit++;
    }
}

GLuint ResourceManager::loadTexture(std::string filepath, int flags)
{
    std::map<std::string, GLuint>::iterator mit;
    mit=textureMap.find(filepath);
    if(mit==textureMap.end())
    {
        GLuint textureID;
        glGenTextures(1, &textureID); // Generate 1 unique texture IDs to use
        glBindTexture(GL_TEXTURE_2D, textureID); // Activate texture
        glfwLoadTexture2D(filepath.c_str(), flags); // Load image

        textureMap[filepath]=textureID;
        return textureID;
    }
    else
    {
        return mit->second;
    }
}

GLuint ResourceManager::createTextureFromImage(std::string key, GLFWimage *img, int flags)
{
    std::map<std::string, GLuint>::iterator mit;
    mit=textureMap.find(key);
    if(mit==textureMap.end())
    {
        GLuint textureID;
        glGenTextures(1, &textureID); // Generate 1 unique texture IDs to use
        glBindTexture(GL_TEXTURE_2D, textureID); // Activate texture
        glfwLoadTextureImage2D(img,flags); // Load image

        textureMap[key]=textureID;
        return textureID;
    }
    else
    {
        return mit->second;
    }
}

MeshObject* ResourceManager::loadMeshObject(std::string filepath)
{
    std::map<std::string, MeshObject*>::iterator mit;
    mit=meshMap.find(filepath);
    if(mit==meshMap.end())
    {
        MeshObject* obj = new MeshObject();
        meshMap[filepath]=obj;
        LoadAse(filepath,*obj);
        return obj;
    }
    else
    {
        return mit->second;
    }
}
