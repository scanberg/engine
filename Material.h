#ifndef Material_H
#define Material_H

#include <GL/glfw.h>
#include <string>

/*
Defaultstrukturen för ett material
*/
enum M_TYPE { NONE, DIFF, DIFF_SPEC, DIFF_SPEC_NORM, DIFF_SPEC_NORM_HEIGHT };

typedef struct {
    M_TYPE type;
    GLuint diffuseMap;
    GLuint normalMap;
    GLuint heightMap;
    GLuint specularMap;
    GLuint shader;

    GLfloat ambient[3];
    GLfloat diffuse[3];
    GLfloat specular[3];
    GLfloat shininess[1];
}Material;

void LoadMaterial(std::string s, Material &mat);
void LoadShader(std::string svertex, std::string sfragment, Material &mat);
void setUniformVariables( GLuint programObj,
			   int diff, int norm, int height, const GLvoid* tangentPointer);

#endif
