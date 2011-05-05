#include <GL/glfw.h>
#include <math.h>
#include <iostream>
#include "Camera.h"

Camera* Camera::activeCamera = NULL;

Camera::Camera()
{
    pos = glm::vec3(0.0f);
    rot = glm::vec3(0.0f);

    Camera::setActiveCamera(this);
}

void Camera::setUp()
{

    // Select and setup the modelview matrix.
    glMatrixMode( GL_MODELVIEW ); // "We want to edit the modelview matrix"
    glLoadIdentity(); // Reset the matrix to identity

    Camera* cam = Camera::getActiveCamera();

    glRotatef(-cam->rot.x,1.0,0.0,0.0);
    glRotatef(-cam->rot.y,0.0,1.0,0.0);
    glRotatef(-cam->rot.z,0.0,0.0,1.0);

    glTranslatef(-cam->pos.x,-cam->pos.y,-cam->pos.z); //translate the screen

}

void Camera::setPosition(float _x, float _y, float _z)
{
    pos.x=_x;
    pos.y=_y;
    pos.z=_z;
}

void Camera::setRotation(float _rx, float _ry, float _rz)
{
    rot.x=_rx;
    rot.y=_ry;
    rot.z=_rz;
}
