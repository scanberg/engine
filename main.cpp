#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glfw.h>
#include <iostream>
#include <ctime>

#include "Entity.h"
#include "AseReader.h"
#include "Material.h"
#include "Error.h"
#include "Camera.h"
#include "SceneHandler.h"
#include "MD5Model.h"

using std::cout;
using std::endl;

float fRandom()
{
    return (float)rand() / (float)RAND_MAX;
}

int main(int argc, char *argv[])
{
    srand ( time(NULL) );
    int running = GL_TRUE;
    int mousebtn, lastmousebtn;

    //Did the init not succeed?
    if(!SceneHandler::Init())
    {
        //quit.
        return 0;
    }

    Light *moon = SceneHandler::CreateLight();
    moon->setPosition(-2000.0,0.0,1000.0);
    moon->setDiffuse(0.3,0.3,0.4);
    moon->setAmbient(0.2,0.2,0.3);
    moon->setSpecular(1.0,1.0,1.0);
    moon->setRadius(20000.0);

    int numLights=4;

    Light *lights[numLights];
    ParticleSystem *ps[numLights];

    ps[0] = SceneHandler::CreateParticleSystem();
    ps[0]->SetPosition(-307.0,-10.0,90.0);
    ps[0]->scale = 2.0;

    lights[0] = SceneHandler::CreateLight();
    lights[0]->setPosition(-307.0,-10.0,90.0);
    lights[0]->setDiffuse(1.0,1.0,0.8);
    lights[0]->setAmbient(0.1,0.1,0.1);
    lights[0]->setSpecular(1.0,1.0,1.0);
    lights[0]->setRadius(200.0);

    ps[1] = SceneHandler::CreateParticleSystem();
    ps[1]->SetPosition(-307.0,-188.0,90.0);
    ps[1]->scale = 2.0;

    lights[1] = SceneHandler::CreateLight();
    lights[1]->setPosition(-307.0,-188.0,90.0);
    lights[1]->setDiffuse(1.0,1.0,0.8);
    lights[1]->setAmbient(0.1,0.1,0.1);
    lights[1]->setSpecular(1.0,1.0,1.0);
    lights[1]->setRadius(200.0);

    ps[2] = SceneHandler::CreateParticleSystem();
    ps[2]->SetPosition(178.0,-45.0,64.0);
    ps[2]->scale = 2.0;

    lights[2] = SceneHandler::CreateLight();
    lights[2]->setPosition(178.0,-45.0,64.0);
    lights[2]->setDiffuse(1.0,1.0,0.8);
    lights[2]->setAmbient(0.1,0.1,0.1);
    lights[2]->setSpecular(1.0,1.0,1.0);
    lights[2]->setRadius(200.0);

    ps[3] = SceneHandler::CreateParticleSystem();
    ps[3]->SetPosition(178.0,-131.0,64.0);
    ps[3]->scale = 2.0;

    lights[3] = SceneHandler::CreateLight();
    lights[3]->setPosition(178.0,-131.0,64.0);
    lights[3]->setDiffuse(1.0,1.0,0.8);
    lights[3]->setAmbient(0.1,0.1,0.1);
    lights[3]->setSpecular(1.0,1.0,1.0);
    lights[3]->setRadius(200.0);

    mousebtn = lastmousebtn = GLFW_RELEASE;

    Camera camera;
    camera.setDirection(90.0,0.0,0.0);
    camera.setPosition(0.0,0.0,100.0);

    PlayerEntity *player;

    player = SceneHandler::CreatePlayerEntity();
    player->SetPosition(-400.0,-100.0,50.0);
    player->minBox=glm::vec4(-12.0,-12.0,-30.0,1.0f);
    player->maxBox=glm::vec4(12.0,12.0,30.0,1.0f);
    player->eyeHeight=25.0;

    PlayerEntity::SetCamera(&camera);

    StaticEntity *scene;
    StaticEntity *beast;
    StaticEntity *box;

    scene = SceneHandler::CreateStaticEntity("media/ny.ase",1.0);
    beast = SceneHandler::CreateStaticEntity("media/beast/beast1.ase",0.5);
    box = SceneHandler::CreateStaticEntity("media/box/box.ase",1.0);

    MD5Model *bob;

    bob = SceneHandler::CreateMD5Entity("models/monsters/hellknight/hellknight.md5mesh","models/monsters/hellknight/idle2.md5anim");
    bob->scale=0.7;
    bob->SetPosition(300.0,100.0,0.0);
	bob->SetRotation(0.0,0.0,-90.0);

    beast->SetPosition(0.0,100.0,50.0);
    beast->SetRotation(90.0,0.0,0.0);

    scene->SetPosition(500.0,200.0,0.0);
    scene->SetRotation(0.0,0.0,0.0);

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

//        glm::vec3 minBBoxPoint = box->ClosestMeshBBoxPoint(0);
//        glm::vec3 dist = minBBoxPoint - Camera::getActiveCamera()->pos;
//        std::cout<<std::sqrt(glm::dot(dist,dist))<<std::endl;

        //bob.Update(SceneHandler::g_dt);

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

