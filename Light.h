#ifndef Light_H
#define Light_H

#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glfw.h>
#include "dMath\dVector.h"

#define SHADOW_MAP_WIDTH 512
#define SHADOW_MAP_HEIGHT 512

class Light
{
public:

    GLuint fboId;
    GLuint depthTextureId;

    Light();
    void Update(){}

    void setDiffuse(GLfloat r, GLfloat g, GLfloat b) { diffuse[0]=r; diffuse[1]=g; diffuse[2]=b; }
    void setAmbient(GLfloat r, GLfloat g, GLfloat b) { ambient[0]=r; ambient[1]=g; ambient[2]=b; }
    void setSpecular(GLfloat r, GLfloat g, GLfloat b) { specular[0]=r; specular[1]=g; specular[2]=b; }
    void setPosition(GLfloat x, GLfloat y, GLfloat z) { position[0]=x; position[1]=y; position[2]=z; }
    void setDirection(GLfloat x, GLfloat y, GLfloat z) { direction[0]=x; direction[1]=y; direction[2]=z; }
    void assignTo(unsigned int i);
    void setupMatrices();
    void setTextureMatrix();

    dVector getPosition(){ return dVector(position[0],position[1],position[2],position[3]); }
private:
    GLfloat diffuse[4];
    GLfloat ambient[4];
    GLfloat specular[4];
    GLfloat position[4];
    GLfloat direction[4];
    GLfloat radius;

    void generateShadowFBO();
};

#endif
