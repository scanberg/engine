#include <GL/glfw.h>
#include <math.h>
#include "Camera.h"

fpsCamera::fpsCamera()
{

    x=y=z=0.0;
    rx=ry=rz=0.0;

    glfwGetMousePos( &mousex, &mousey ); // Requires an open window
    lastmousex = mousex;
    lastmousey = mousey;
}

void fpsCamera::setUp()
{
    // Select and setup the modelview matrix.
    glMatrixMode( GL_MODELVIEW ); // "We want to edit the modelview matrix"
    glLoadIdentity(); // Reset the matrix to identity

    glRotatef(rx,1.0,0.0,0.0);
    glRotatef(ry,0.0,1.0,0.0);
    glRotatef(rz,0.0,0.0,1.0);

    glTranslated(-x,-y,-z); //translate the screen
}

void fpsCamera::update()
{
    lastmousex = mousex;
    lastmousey = mousey;

    glfwGetMousePos( &mousex, &mousey );

    rz -= (lastmousex - mousex) * 0.5;
    rx -= (lastmousey - mousey) * 0.5;
    if ( rx < 179.5f ) rx = 179.5f;
    if ( rx > 359.0f ) rx = 359.0f;
}

void fpsCamera::position(float _x, float _y, float _z)
{
    x=_x;
    y=_y;
    z=_z;
}
void fpsCamera::move(float _x, float _y, float _z)
{
    float xrad = rx / 180 * 3.141592654f;
    float yrad = ry / 180 * 3.141592654f;
    float zrad = rz / 180 * 3.141592654f;

    x += _z*float(sin(zrad)) + _x*float(cos(zrad));
    y += _z*float(cos(zrad)) - _x*float(sin(zrad));
    //z -= _z*float(cos(xrad)) ;
}
void fpsCamera::rotate(float _rx, float _ry, float _rz)
{
    rx=_rx;
    ry=_ry;
    rz=_rz;
}
