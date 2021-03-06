#ifndef Material_H
#define Material_H

#include <GL/glfw.h>
#include <string>

/*
Defaultstrukturen f�r ett material
*/
const short TEX_NONE        = 0;
const short TEX_DIFFUSE     = 1;
const short TEX_NORMAL      = 2;
const short TEX_HEIGHT      = 4;
const short TEX_SPECULAR    = 8;

class Material
{
public:
    short type;
    GLuint diffuseMap;
    GLuint normalMap;
    GLuint heightMap;
    GLuint specularMap;
    GLuint shader;

    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat shininess[1];

    Material();

    void setAmbient(float f);
    void setAmbient(float r, float g, float b);

    void setDiffuse(float f);
    void setDiffuse(float r, float g, float b);

    void setSpecular(float f);
    void setSpecular(float r, float g, float b);

    void setShininess(float f);

    void loadShader();
};

GLFWimage mergeRGB_A(GLuint *rgb_tex, GLuint *a_tex);
GLFWimage mergeRGB_A(std::string *rgbFile, std::string *aFile);

GLuint createShader( const char *vertfilename, const char *fragfilename );

#endif
