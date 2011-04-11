#ifndef Material_H
#define Material_H

#include <GL/glfw.h>
#include <string>

using std::string;

/*
Defaultstrukturen för ett material
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

    GLfloat ambient[3];
    GLfloat diffuse[3];
    GLfloat specular[3];
    GLfloat shininess[1];

    Material();

    void setAmbient(float f);
    void setAmbient(float r, float g, float b);

    void setDiffuse(float f);
    void setDiffuse(float r, float g, float b);

    void setSpecular(float f);
    void setSpecular(float r, float g, float b);

    void setShininess(float f);
};

Material LoadMaterial(const string& s);
int LoadMaterial(const string& s, Material &mat);

void setUniformVariables( GLuint programObj,
			   int diff, int norm, int height, const GLvoid* tangentPointer);

#endif
