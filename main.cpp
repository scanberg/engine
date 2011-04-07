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

using namespace std;

double t0 = 0.0;
float t = 0.0;
int frames = 0;
char titlestring[200];
GLuint textureID;

/*
 * Show the fps in the windowframe /Stegu
 */
void showFPS()
{

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

/*
 * supportsOpenGLVersion - make sure OpenGL version is recent enough.
 * (Function copied from NVidia's OpenGL developer documentation)
 */
int supportsOpenGLVersion( int atLeastMajor, int atLeastMinor )
{
    const char *version;
    int major, minor;
    version = (const char *)glGetString( GL_VERSION );
    if ( sscanf( version, "%d.%d", &major, &minor ) == 2 ) {
        if ( major > atLeastMajor )
            return 1;
        if ( ( major == atLeastMajor ) && ( minor >= atLeastMinor ) )
            return 1;
    } else {
        printError( "Malformed OpenGL version string", version );
    }
    return 0;
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

    //Did the init not succeed?
    if(!init())
    {
        //quit.
        return 0;
    }

    // Enable back face culling and Z buffering
    glEnable(GL_CULL_FACE); // Cull away all back facing polygons
    glEnable(GL_DEPTH_TEST); // Use the Z buffer

    Entity scene;
    StaticEntity object;

    Mesh model;
    LoadAse("media/box/box.ase",model);
    model.CalculateNormals();
    model.CreateBuffers();
    object.mesh=&model;

    Material mat;
    LoadMaterial("media/material/rockwall/rockwall",mat);

    object.material=&mat;
    LoadShader("shaders/vertex_parallax.glsl","shaders/fragment_parallax.glsl",mat);

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

    object.SetPosition(0.0,50.0,5.0);
    scene.AddChild(object);

    scene.SetPosition(0.0,0.0,0.0);

    glfwSwapInterval(0); // Do not wait for screen refresh between frames

    // Main loop
    while(running)
    {
        showFPS();

        t = (float)glfwGetTime();

        glClearColor( 0.2f, 0.2f, 0.2f, 0.0f );
        // Clear the color buffer and the depth buffer.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwGetWindowSize( &width, &height );
        if(height<1) height=1; // Safeguard against iconified/closed window

        handleResize();
        setupCamera();

        // Update object.
        object.SetRotation(0.0,0.0,15.0*t);

        float vector4f[4]= {0.0f, -100.0f, 50.0f, 1.0f}; // Origin, in hom. coords
        glLightfv(GL_LIGHT0, GL_POSITION, vector4f); // Set light position

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
