#ifndef Light_H
#define Light_H

#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glfw.h>
#include "Math.h"

#define SHADOW_MAP_WIDTH 1024
#define SHADOW_MAP_HEIGHT 1024

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
    void setCutoff(GLfloat c) {cutoff=c;}
    void assignTo(unsigned int i);
    void setupMatrices();
    void setTextureMatrix();

    glm::vec4 getPosition(){ return glm::vec4(position[0],position[1],position[2],position[3]); }
    glm::vec4 getDirection(){ return glm::vec4(direction[0],direction[1],direction[2],direction[3]); }
    glm::vec4 getDiffuse(){ return glm::vec4(diffuse[0],diffuse[1],diffuse[2],diffuse[3]); }

    GLfloat getRadius(){ return radius; };

private:
    GLfloat diffuse[4];
    GLfloat ambient[4];
    GLfloat specular[4];
    GLfloat position[4];
    GLfloat direction[4];
    GLfloat radius;
    GLfloat cutoff;

    void generateShadowFBO();
};

#endif
