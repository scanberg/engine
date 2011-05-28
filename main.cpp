#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glfw.h>
#include <iostream>

#include "Entity.h"
#include "AseReader.h"
#include "Material.h"
#include "Error.h"
#include "Camera.h"
#include "SceneHandler.h"
#include "MD5Model.h"

using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    int running = GL_TRUE;
    int mousebtn, lastmousebtn;

    //Did the init not succeed?
    if(!SceneHandler::Init())
    {
        //quit.
        return 0;
    }

    int numLights=4;
    Light *lights[numLights];

    for(int i=0; i<numLights; i++)
    {
        lights[i] = SceneHandler::CreateLight();
        lights[i]->setPosition(0.0,0.0,0.0);
        lights[i]->setDirection(0.0,0.0,0.0);
        lights[i]->setDiffuse(1.0,1.0,1.0);
        lights[i]->setAmbient(0.1,0.1,0.1);
        lights[i]->setSpecular(1.0,1.0,1.0);
        lights[i]->setRadius(300.0);
    }

    mousebtn = lastmousebtn = GLFW_RELEASE;

    Camera camera;
    camera.setDirection(90.0,0.0,0.0);
    camera.setPosition(0.0,0.0,100.0);

    PlayerEntity *player;

    player = SceneHandler::CreatePlayerEntity();
    player->SetPosition(0.0,-100.0,50.0);
    player->minBox=glm::vec4(-12.0,-12.0,-30.0,1.0f);
    player->maxBox=glm::vec4(12.0,12.0,30.0,1.0f);
    player->eyeHeight=25.0;

    PlayerEntity::SetCamera(&camera);

    StaticEntity *scene;
    StaticEntity *beast;
    StaticEntity *box;

    scene = SceneHandler::CreateStaticEntity("media/testscene/testscene.ase",10.0);
    beast = SceneHandler::CreateStaticEntity("media/beast/beast1.ase",0.5);
    box = SceneHandler::CreateStaticEntity("media/box/box.ase",1.0);

	ParticleSystemEntity *particlesystem;
	particlesystem = SceneHandler::CreateParticleSystem();

    //MD5Model bob;
    //bob.LoadModel("md5/monsters/hellknight/hellknight.md5mesh");
    //bob.LoadAnim("md5/monsters/hellknight/idle2.md5anim");

    beast->SetPosition(0.0,100.0,50.0);
    beast->SetRotation(90.0,0.0,0.0);

    scene->SetPosition(0.0,0.0,0.0);
    scene->SetRotation(0.0,0.0,45.0);

    box->SetPosition(100.0,100.0,150.0);
    box->SetRotation(0.0,0.0,-20.0);

    SceneHandler::CreateBBoxCollision(box,10.0);
    SceneHandler::CreateBBoxCollision(beast,10.0);
    SceneHandler::CreateMeshCollision(scene);

    SceneHandler::CreatePlayerCollision(player);

    GLint tex_units;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &tex_units);
    cout<<"Max Texture units: "<<tex_units<<endl;

    glfwSwapInterval(0); // Do not wait for screen refresh between frames

    GLuint errorID = glGetError();
    if(errorID != GL_NO_ERROR) {
        printf("\nOpenGL error: %s\n", gluErrorString(errorID));
        printf("Attempting to proceed anyway. Expect rendering errors or a crash.\n");
    }

    // Main loop
    while(running)
    {

        mousebtn=glfwGetMouseButton( GLFW_MOUSE_BUTTON_1 );

        if(mousebtn == GLFW_PRESS)
        {
            glfwDisable( GLFW_MOUSE_CURSOR );
            PlayerEntity::followMouse=true;
        }

        mousebtn=glfwGetMouseButton( GLFW_MOUSE_BUTTON_2 );
        if(mousebtn == GLFW_PRESS)
        {
            glfwEnable( GLFW_MOUSE_CURSOR );
            PlayerEntity::followMouse=false;
        }

        // Update the scene.
        SceneHandler::Update();
        //bob.Update(SceneHandler::g_dt);
        static float t;
        t+=SceneHandler::g_dt;

        for(int i=0; i<numLights; i++)
        {
            lights[i]->setPosition(250.0*cos(t*0.1*(i+1)),250.0*sin(t*0.2*(i+1)),50.0+50*sin(t*0.3*(i+1)));
        }


        camera.setUp();

        // Finally, draw the scene.
        SceneHandler::Render();
        //bob.Render();

        // Swap buffers, i.e. display the image and prepare for next frame.
        glfwSwapBuffers();

        // Check if the ESC key was pressed or the window was closed.
        if(glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED))
            running = GL_FALSE;
    }

    // Free the allocated resources
    SceneHandler::Destroy();

    return(0);
}

