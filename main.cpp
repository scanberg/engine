#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glfw.h>

#include <iostream>
#include <cstdio>
#include <math.h>
#include "Entity.h"
#include "AseReader.h"
#include "Material.h"
#include "Error.h"
#include "Camera.h"

using namespace std;

char titlestring[200];

/*
 * Show the fps in the windowframe /Stegu
 */
void showFPS()
{
    static double t0 = 0.0;
    static int frames = 0;
    double t, fps;

    // Get current time
    t = glfwGetTime();  // Gets number of seconds since glfwInit()
    // If one second has passed, or if this is the very first frame

    if( (t-t0) > 1.0 || frames == 0 )
    {
        fps = (double)frames / (t-t0);
        sprintf(titlestring, "FPS: %.2f",fps);
        glfwSetWindowTitle(titlestring);
        t0 = t;
        frames = 0;
    }
    frames ++;
}

/*
 * Handle the resizing of the window. /Stegu
 */
void handleResize()
{

    int width, height;

    // Get window size. It may start out different from the requested
    // size, and will change if the user resizes the window.
    glfwGetWindowSize( &width, &height );
    if(height<=0) height=1; // Safeguard against iconified/closed window

    // Set viewport. This is the pixel rectangle we want to draw into.
    glViewport( 0, 0, width, height ); // The entire window

    // Select and setup the projection matrix.
    glMatrixMode(GL_PROJECTION); // "We want to edit the projection matrix"
    glLoadIdentity(); // Reset the matrix to identity
    // 65 degrees FOV, same aspect ratio as window, depth range 1 to 100
    gluPerspective( 65.0f, (GLfloat)width/(GLfloat)height, 1.0f, 1000.0f );

}

void setupCamera()
{

    // Select and setup the modelview matrix.
    glMatrixMode( GL_MODELVIEW ); // "We want to edit the modelview matrix"
    glLoadIdentity(); // Reset the matrix to identity

    // Look from 0,-5,0 towards 0,0,0 with Z as "up" in the image
    gluLookAt( 0.0, -10.0, 0.0,    // Eye position
               0.0f, 0.0f, 0.0f,   // View point
               0.0f, 0.0f, 1.0f );  // Up vector

}

int init()
{
    // Initialise GLFW
    glfwInit();

    // Open the OpenGL window
    if( !glfwOpenWindow(640, 480, 8,8,8,8, 32,0, GLFW_WINDOW) )
    {
        glfwTerminate(); // glfwOpenWindow failed, quit the program.
        printError( "Error", "glfw could not open a window." );
        return 0;
    }

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return 0;
    }
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

    if (!GLEW_VERSION_2_0)
    {
        printError( "Error", "OpenGL version 2.0 or later is required." );
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[])
{
    int width, height;
    int running = GL_TRUE; // Main loop exits when this is set to GL_FALSE
    float speed, xspeed, zspeed;
    int mousebtn, lastmousebtn;

    float lastt=0.0;
    float t, dt;

    //Did the init not succeed?
    if(!init())
    {
        //quit.
        return 0;
    }

    mousebtn = lastmousebtn = GLFW_RELEASE;

    // Enable back face culling and Z buffering
    glEnable(GL_CULL_FACE); // Cull away all back facing polygons
    glEnable(GL_DEPTH_TEST); // Use the Z buffer

    fpsCamera camera;
    camera.rotate(-180.0,0.0,45.0);

    Entity scene;
    scene.SetName("SCENE");
    StaticEntity object;
    object.SetName("OBJECT");

    StaticEntity plane;

    LoadAse("media/box/boxes.ase",object);

    Material mat;
    mat = LoadMaterial("media/material/rockwall/rockwall");

    LoadAse("media/plane.ase",plane);
    plane.SetName("OBJECT");

    plane.material=&mat;


    /*
    for(unsigned int xx=0; xx<model.numVertices; xx++)
    {
        cout<<"Tang "<<xx<<" :"<<"( "<<model.tangent[xx].x<<", "<<model.tangent[xx].y<<", "<<model.tangent[xx].x<<" )"<<endl;
        cout<<"Vert "<<xx<<" :"<<"( "<<model.vertex[xx].nx<<", "<<model.vertex[xx].ny<<", "<<model.vertex[xx].nx<<" )"<<endl;
    }
    */

    GLint tex_units;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &tex_units);
    cout<<"Max Texture units: "<<tex_units<<endl;

    scene.AddChild(object);
    scene.AddChild(plane);

    scene.SetPosition(0.0,0.0,0.0);
    object.SetPosition(-5.0,50.0,5.0);

    plane.SetPosition(0.0,100.0,-50.0);
    plane.SetRotation(-90.0,0.0,0.0);

    glfwSwapInterval(0); // Do not wait for screen refresh between frames

    GLuint errorID = glGetError();
    if(errorID != GL_NO_ERROR) {
        printf("\nOpenGL error: %s\n", gluErrorString(errorID));
        printf("Attempting to proceed anyway. Expect rendering errors or a crash.\n");
    }

    // Main loop
    while(running)
    {
        showFPS();

        t = (float)glfwGetTime();
        dt = t - lastt;
        lastt = t;

        speed=50.0*dt;

        mousebtn=glfwGetMouseButton( GLFW_MOUSE_BUTTON_1 );

        if(mousebtn == GLFW_PRESS)
        {
            glfwEnable( GLFW_MOUSE_CURSOR );
            camera.setFollowMouse(false);
        }

        mousebtn=glfwGetMouseButton( GLFW_MOUSE_BUTTON_2 );
        if(mousebtn == GLFW_PRESS)
        {
            glfwDisable( GLFW_MOUSE_CURSOR );
            camera.setFollowMouse(true);
        }

        glClearColor( 0.2f, 0.2f, 0.2f, 0.0f );
        // Clear the color buffer and the depth buffer.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwGetWindowSize( &width, &height );
        if(height<1) height=1; // Safeguard against iconified/closed window

        camera.update();

        zspeed=0.0f;
        xspeed=0.0f;

        if(glfwGetKey('W'))
            zspeed += 1.0f;
        if(glfwGetKey('S'))
            zspeed -= 1.0f;
        if(glfwGetKey('A'))
            xspeed -= 1.0f;
        if(glfwGetKey('D'))
            xspeed += 1.0f;

        zspeed*=speed;
        xspeed*=speed;

        camera.move(xspeed,0.0,zspeed);

        handleResize();

        camera.setUp();

        // Update object.
        object.SetRotation(0.0,0.0,15.0*t);

        float vector4f[4]= {0.0f, -100.0f, 50.0f, 1.0f}; // Origin, in hom. coords
        glLightfv(GL_LIGHT0, GL_POSITION, vector4f); // Set light position

        float ambient[4]= {0.3f, 0.3f, 0.3f, 1.0f}; // Origin, in hom. coords
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient); // Set light position

        // Finally, draw the scene.
        scene.Draw();

        // Swap buffers, i.e. display the image and prepare for next frame.
        glfwSwapBuffers();

        // Check if the ESC key was pressed or the window was closed.
        if(glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED))
            running = GL_FALSE;
    }

    return(0);
}
