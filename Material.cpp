#define GLEW_STATIC
#include <GL/glew.h>

#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdio.h>   // for printf() and sscanf()
#include <stdlib.h>  // for malloc() and free()
#include "Material.h"
#include "Error.h"
#include "SceneHandler.h"

using std::string;

Material::Material()
{
    //Tilldela standardvärden
    ambient[3]=diffuse[3]=specular[3]=1.0f;
    setAmbient(0.2);
    setDiffuse(1.0);
    setSpecular(0.2);
    setShininess(50.0);
    shader=0;
    type=TEX_NONE;
}

void Material::setAmbient(float f) { setAmbient(f,f,f); }

void Material::setAmbient(float r, float g, float b)
{
    this->ambient[0]=r;
    this->ambient[1]=g;
    this->ambient[2]=b;
}

void Material::setDiffuse(float f) { setDiffuse(f,f,f); }

void Material::setDiffuse(float r, float g, float b)
{
    this->diffuse[0]=r;
    this->diffuse[1]=g;
    this->diffuse[2]=b;
}

void Material::setSpecular(float f) { setSpecular(f,f,f); }

void Material::setSpecular(float r, float g, float b)
{
    this->specular[0]=r;
    this->specular[1]=g;
    this->specular[2]=b;
}

void Material::setShininess(float f)
{
    this->shininess[0]=f;
}

//bool FileExists(std::string strFilename);
GLFWimage mergeNormalAndHeight(std::string s);

void Material::loadShader()
{
    //Ladda rätt shader beroende på vilka texturer som hittades.

    shader=SceneHandler::shaderLib.GetShaderFromType(type);
    std::cout<<"shader: "<<shader;
}

/*
    *** mergeNormalAndHeight ***

    Slår samman Normal och Height -map.
    Normal fyller RGB kanalerna och Height lagras i A kanalen.

    TODO:
    Kontrollera så att Width och Height är lika för normal och height.
*/
GLFWimage mergeRGB_A(std::string *rgbFile, std::string *aFile)
{
    GLFWimage img;
    GLFWimage normImg;
    GLFWimage heightImg;

    glfwReadImage(rgbFile->c_str(), &normImg, GLFW_NO_RESCALE_BIT); //GLFW_NO_RESCALE_BIT

    glfwReadImage(aFile->c_str(), &heightImg, GLFW_NO_RESCALE_BIT);

    unsigned char *rgbaData = new unsigned char[normImg.Width * normImg.Height * 4];

    for(int i=0; i<(normImg.Width * normImg.Height); i++)
    {
        rgbaData[4*i]   = normImg.Data[3*i];    //R
        rgbaData[4*i+1] = normImg.Data[3*i+1];  //G
        rgbaData[4*i+2] = normImg.Data[3*i+2];  //B
        rgbaData[4*i+3] = heightImg.Data[i];    //A
    }

    img.Format = GL_RGBA;
    img.BytesPerPixel = 4;
    img.Width = normImg.Width;
    img.Height = normImg.Height;
    img.Data = rgbaData;

    glfwFreeImage(&heightImg);
    glfwFreeImage(&normImg);

    return img;
}

GLFWimage mergeRGB_A(GLuint *rgb_tex, GLuint *a_tex)
{
    GLFWimage img;
    int width, height;

    glBindTexture(GL_TEXTURE_2D, *rgb_tex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    GLubyte *rgbData = new GLubyte[width*height*3];
    glGetTexImage(	GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbData);

    glBindTexture(GL_TEXTURE_2D, *a_tex);
    GLubyte *aData = new GLubyte[width*height];
    glGetTexImage(	GL_TEXTURE_2D, 0, GL_R, GL_UNSIGNED_BYTE, aData);

    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned char *rgbaData = new unsigned char[ width*height*4 ];

    for(int i=0; i<(width*height); i++)
    {
        rgbaData[4*i]   = rgbData[3*i];     //R
        rgbaData[4*i+1] = rgbData[3*i+1];   //G
        rgbaData[4*i+2] = rgbData[3*i+2];   //B
        rgbaData[4*i+3] = aData[i];         //A
    }

    img.Format = GL_RGBA;
    img.BytesPerPixel = 4;
    img.Width = width;
    img.Height = height;
    img.Data = rgbaData;

    return img;
}
