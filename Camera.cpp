#include <GL/glfw.h>
#include <math.h>
#include <iostream>
#include "Camera.h"

Camera* Camera::activeCamera = NULL;

Camera::Camera()
{
    pos = glm::vec3(0.0f);
    dir = glm::vec3(0.0f);

    Camera::setActiveCamera(this);
}

void Camera::setUp()
{

    // Select and setup the modelview matrix.
    glMatrixMode( GL_MODELVIEW ); // "We want to edit the modelview matrix"
    glLoadIdentity(); // Reset the matrix to identity

    Camera* cam = Camera::getActiveCamera();

    glRotatef(-cam->dir.x,1.0,0.0,0.0);
    glRotatef(-cam->dir.y,0.0,1.0,0.0);
    glRotatef(-cam->dir.z,0.0,0.0,1.0);

    glTranslatef(-cam->pos.x,-cam->pos.y,-cam->pos.z); //translate the screen

}

void Camera::setUpInverse()
{
    // Select and setup the modelview matrix.
    glMatrixMode( GL_MODELVIEW ); // "We want to edit the modelview matrix"
    glLoadIdentity(); // Reset the matrix to identity

    Camera* cam = Camera::getActiveCamera();

    glTranslatef(cam->pos.x,cam->pos.y,cam->pos.z); //translate the screen

    glRotatef(cam->dir.x,1.0,0.0,0.0);
    glRotatef(cam->dir.y,0.0,1.0,0.0);
    glRotatef(cam->dir.z,0.0,0.0,1.0);
}

glm::mat4 Camera::getViewMatrix()
{
    Camera* cam = Camera::getActiveCamera();

    glm::mat4 rotx = glm::gtc::matrix_transform::rotate(glm::mat4(1.0f),-cam->dir.x,glm::vec3(1.0f,0.0f,0.0f));
    glm::mat4 roty = glm::gtc::matrix_transform::rotate(glm::mat4(1.0f),-cam->dir.y,glm::vec3(0.0f,1.0f,0.0f));
    glm::mat4 rotz = glm::gtc::matrix_transform::rotate(glm::mat4(1.0f),-cam->dir.z,glm::vec3(0.0f,0.0f,1.0f));
    glm::mat4 trans = glm::gtc::matrix_transform::translate(glm::mat4(1.0),-cam->pos);

    glm::mat4 rot = rotx*roty*rotz;

    glm::mat4 mat = rot*trans;

    return mat;
}

glm::mat4 Camera::getInvViewMatrix()
{
    Camera* cam = Camera::getActiveCamera();
    float c = 3.14158f/180.0f;
    glm::mat4 rotx = glm::gtc::matrix_transform::rotate(glm::mat4(1.0f),cam->dir.x,glm::vec3(1.0f,0.0f,0.0f));
    glm::mat4 roty = glm::gtc::matrix_transform::rotate(glm::mat4(1.0f),cam->dir.y,glm::vec3(0.0f,1.0f,0.0f));
    glm::mat4 rotz = glm::gtc::matrix_transform::rotate(glm::mat4(1.0f),cam->dir.z,glm::vec3(0.0f,0.0f,1.0f));
    glm::mat4 trans = glm::gtc::matrix_transform::translate(glm::mat4(1.0),cam->pos);

    return rotz*roty*rotx*trans;
}

void Camera::setPosition(float _x, float _y, float _z)
{
    pos.x=_x;
    pos.y=_y;
    pos.z=_z;
}

void Camera::setDirection(float _rx, float _ry, float _rz)
{
    dir.x=_rx;
    dir.y=_ry;
    dir.z=_rz;
}
