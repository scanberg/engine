#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glfw.h>
#include <iostream>
#include <stdlib.h>
#include <cstdio>
#include <Newton.h>
#include <JointLibrary.h>

#include "MD5Model.h"
#include "SceneHandler.h"
#include "Error.h"
#include "Entity.h"
#include "AseReader.h"
#include "Physics.h"
#include "Material.h"

#define USE_VISUAL_DEBUGGER

#define PHYSICS_FPS	                60.0f
#define FPS_IN_MICROSECONDS         (int (1000000.0f/PHYSICS_FPS))
#define MAX_PHYSICS_LOOPS		    1

using namespace std;

static int g_currentTime;
static int g_timeAccumulator = FPS_IN_MICROSECONDS;

//static void* g_newtonDebugger;

void* AllocMemory (int sizeInBytes);
void FreeMemory (void *ptr, int sizeInBytes);
void AdvanceSimulation (int timeInMilisecunds);
int GetTimeInMicroseconds (){ return (int)(glfwGetTime()*1.0e6); }

//#pragma warning (disable: 4100) //unreferenced formal parameter
//#pragma warning (disable: 4702) //unreachable code

static char titlestring[200];

// Allocate statics.

vector<Entity*> SceneHandler::entity;
vector<NewtonEntity*> SceneHandler::newtonEntity;
vector<Entity*> SceneHandler::renderList;
vector<Light*> SceneHandler::light;
NewtonWorld* SceneHandler::world;
int SceneHandler::width;
int SceneHandler::height;
int SceneHandler::near;
int SceneHandler::far;
float SceneHandler::g_dt;
float SceneHandler::interpolationParam;
DeferredFBO SceneHandler::deferred;
GLuint SceneHandler::shadowShader;
ResourceManager SceneHandler::resources;
ShaderLibrary SceneHandler::shaderLib;

/*
 * Handle the resizing of the window. /Stegu
 */
void handleResize()
{

    // Get window size. It may start out different from the requested
    // size, and will change if the user resizes the window.
    int width, height;
    glfwGetWindowSize( &width, &height );

    if(height<=0) height=1; // Safeguard against iconified/closed window

    if(SceneHandler::width!=width || SceneHandler::height!=height)
    {
        SceneHandler::width=width;
        SceneHandler::height=height;
        SceneHandler::InitDeferred(width,height);
    }

    // Set viewport. This is the pixel rectangle we want to draw into.
    glViewport( 0, 0, SceneHandler::width, SceneHandler::height ); // The entire window
}

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

void updateDt()
{
    static float t0 = 0.0;
    double t;
    t = glfwGetTime();
    SceneHandler::g_dt=t-t0;
    t0=t;
}

void setupMatrices()
{
    glMatrixMode(GL_PROJECTION); // "We want to edit the projection matrix"
    glLoadIdentity(); // Reset the matrix to identity
    gluPerspective( 65.0f, (GLfloat)SceneHandler::width/(GLfloat)SceneHandler::height, (GLfloat)SceneHandler::near, (GLfloat)SceneHandler::far);

    Camera::setUp();
}


void SceneHandler::GenerateShadowMaps()
{
    for(unsigned int i=0; i<SceneHandler::light.size(); i++)
    {
        //First step: Render from the light POV to a FBO, story depth values only
        glBindFramebuffer(GL_FRAMEBUFFER,light.at(i)->fboId);	//Rendering offscreen

        //Using the fixed pipeline to render to the depthbuffer
        glUseProgram(0);

        // In the case we render the shadowmap to a higher resolution, the viewport must be modified accordingly.
        glViewport(0,0,SHADOW_MAP_WIDTH,SHADOW_MAP_HEIGHT);

        // Clear previous frame values
        glClear( GL_DEPTH_BUFFER_BIT);

        //Disable color rendering, we only want to write to the Z-Buffer
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        light.at(i)->setupMatrices();
        //setupMatrices();

        // Culling switching, rendering only backface, this is done to avoid self-shadowing
        glCullFace(GL_FRONT);

        for(unsigned int u=0; u<SceneHandler::renderList.size(); u++)
        {
            renderList.at(u)->DrawGeometry();
        }

        //Save modelview/projection matrice into texture3, also add a biais
        light.at(i)->setTextureMatrix();
    }
}

int SceneHandler::Init()
{
    // Initialise GLFW
    glfwInit();

    SceneHandler::width=640;
    SceneHandler::height=480;
    SceneHandler::near=5;
    SceneHandler::far=500;

    // Open the OpenGL window
    if( !glfwOpenWindow(SceneHandler::width, SceneHandler::height, 8,8,8,8, 32,0, GLFW_WINDOW) )
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

    // Enable back face culling and Z buffering
    glEnable(GL_CULL_FACE); // Cull away all back facing polygons
    glEnable(GL_DEPTH_TEST); // Use the Z buffer

	// set the memory allocators
	NewtonSetMemorySystem (AllocMemory, FreeMemory);

	// create the Newton World
	SceneHandler::world = NewtonCreate ();

	// use the standard x87 floating point model
	NewtonSetPlatformArchitecture (SceneHandler::world, 0);

	NewtonSetSolverModel (SceneHandler::world, 1);

	// set a fix world size
	glm::vec3 minSize (-2000.0f, -2000.0f, -2000.0f);
	glm::vec3 maxSize ( 2000.0f,  2000.0f,  2000.0f);
	NewtonSetWorldSize (SceneHandler::world, &minSize[0], &maxSize[0]);

	InitDeferred(width,height);

	deferred.shaderFirstPass=shaderLib.LoadShader("shaders/vertex_firstpass.glsl", "shaders/fragment_firstpass.glsl");
	deferred.shaderSecondPass=shaderLib.LoadShader("shaders/vertex_secondpass.glsl", "shaders/fragment_secondpass.glsl");

	shaderLib.Init();

    return 1;
}

void SceneHandler::InitDeferred(GLint w, GLint h)
{
    glDeleteFramebuffers(1,&deferred.fbo);
    glDeleteTextures(1,&deferred.colorMap);
    glDeleteTextures(1,&deferred.normalMap);
    glDeleteTextures(1,&deferred.depthMap);
    glDeleteTextures(1,&deferred.positionMap);

    // generate namespace for the frame buffer, colorbuffer and depthbuffer
    glGenFramebuffers(1, &deferred.fbo);
    glGenTextures(1,&deferred.colorMap);
    glGenTextures(1,&deferred.normalMap);
    glGenTextures(1,&deferred.depthMap);
    glGenTextures(1,&deferred.positionMap);

    deferred.width=w;
    deferred.height=h;

    //switch to our fbo so we can bind stuff to it
    glBindFramebuffer(GL_FRAMEBUFFER, deferred.fbo);

    //create the colorbuffer texture and attach it to the frame buffer
    glBindTexture(GL_TEXTURE_2D, deferred.colorMap);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, deferred.width, deferred.height, 0,GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, deferred.colorMap, 0);

    glBindTexture(GL_TEXTURE_2D, deferred.normalMap);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, deferred.width, deferred.height, 0,GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D, deferred.normalMap, 0);

    glBindTexture(GL_TEXTURE_2D, deferred.depthMap);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, deferred.width, deferred.height, 0,GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D, deferred.depthMap, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Go back to regular frame buffer rendering
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneHandler::Update()
{
    unsigned int i;

    showFPS();

    updateDt();

    AdvanceSimulation(GetTimeInMicroseconds());

    for(i=0; i<SceneHandler::light.size(); i++)
    {
        SceneHandler::light.at(i)->Update();
    }

    for(i=0; i<SceneHandler::entity.size(); i++)
    {
        SceneHandler::entity.at(i)->Update();
    }

    for(i=0; i<SceneHandler::newtonEntity.size(); i++)
    {
        SceneHandler::newtonEntity.at(i)->Update(interpolationParam, SceneHandler::world);
    }
}

void SceneHandler::DrawLights()
{
    glPointSize( 10.0f );

    glPushAttrib( GL_ENABLE_BIT );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable( GL_LIGHTING );
    glEnable( GL_DEPTH_TEST );

    glUseProgram( 0 );

    glBegin( GL_POINTS );
        for ( unsigned int i = 0; i<light.size(); i++)
        {
            glColor3fv( glm::value_ptr(light.at(i)->getDiffuse3()) );
            glVertex3fv( glm::value_ptr(light.at(i)->getPosition3()) );
        }
    glEnd();

    glColor3f( 0.0f, 1.0f, 0.0f );
    glBegin( GL_LINES );
        for ( unsigned int i = 0; i < light.size(); i++ )
        {
            glVertex3fv( glm::value_ptr(light.at(i)->getPosition3()) );
            glVertex3fv( glm::value_ptr(light.at(i)->getPosition3()+light.at(i)->getDirection3()) );
        }
    glEnd();

    glPopAttrib();
}

void DeferredFBO::DrawFirstPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT);

    glViewport(0,0,width, height);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, buffers);

    glClearColor (0.2f, 0.2f, 0.2f, 1.0f);
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

    for(unsigned int i=0; i<SceneHandler::renderList.size(); i++)
    {
        SceneHandler::renderList.at(i)->DrawFirstPass();
    }

    glPopAttrib();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneHandler::getLightVar(int size, glm::vec3 *lightPos, glm::vec3 *lightCol, GLfloat *invLightRadiusSqr)
{
    glm::mat4 mat;
    glGetFloatv(GL_MODELVIEW_MATRIX, &mat[0][0]);

    glm::vec4 pos4;

    for(int i=0; i<size; i++)
    {
        pos4 = SceneHandler::light.at(i)->getPosition4();
        lightPos[i] = glm::vec3(mat * pos4);
        lightCol[i] = SceneHandler::light.at(i)->getDiffuse3();
        invLightRadiusSqr[i] = 1.0 / ( SceneHandler::light.at(i)->getRadius() * SceneHandler::light.at(i)->getRadius() );
    }
}

void DeferredFBO::DrawSecondPass()
{
    //glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);

    int num = SceneHandler::light.size();

    if(num>0)
    {
        glm::vec3 lightPosition[num];
        glm::vec3 lightColor[num];
        GLfloat invLightRadiusSqr[num];

        SceneHandler::getLightVar(num,lightPosition,lightColor,invLightRadiusSqr);

        setUniform1i(shaderSecondPass,num,"numLights");
        setUniform3fv(shaderSecondPass,num,&lightPosition[0][0],"lightPosition");
        setUniform3fv(shaderSecondPass,num,&lightColor[0][0],"lightColor");
        setUniform1fv(shaderSecondPass,num,&invLightRadiusSqr[0],"invLightRadiusSqr");
    }

    glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT);
    glViewport(0,0,width, height);

    glDisable( GL_LIGHTING );
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
    glClear( GL_COLOR_BUFFER_BIT );

    Camera *cam=Camera::getActiveCamera();

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, SceneHandler::deferred.colorMap);
    setUniform1i(shaderSecondPass,0,"colorMap");

    glActiveTexture( GL_TEXTURE1 );
    glBindTexture(GL_TEXTURE_2D, SceneHandler::deferred.normalMap);
    setUniform1i(shaderSecondPass,1,"normalMap");

    glActiveTexture( GL_TEXTURE2 );
    glBindTexture(GL_TEXTURE_2D, SceneHandler::deferred.depthMap);
    setUniform1i(shaderSecondPass,2,"depthMap");

    setUniform2f(shaderSecondPass,SceneHandler::deferred.width,SceneHandler::deferred.height,"screenSize");
    setUniform2f(shaderSecondPass,SceneHandler::near,SceneHandler::far,"cameraRange");
    setUniform3f(shaderSecondPass,cam->pos.x,cam->pos.y,cam->pos.z,"cameraPos");

    glUseProgram( shaderSecondPass );

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(-1, -1);
    glTexCoord2f(1, 0);
    glVertex2f(1, -1);
    glTexCoord2f(1, 1);
    glVertex2f(1, 1);
    glTexCoord2f(0, 1);
    glVertex2f(-1, 1);
    glEnd();

//    glMatrixMode(GL_PROJECTION);
//    glPushMatrix();
//    glLoadIdentity();
//    glMatrixMode(GL_MODELVIEW);
//    glPushMatrix();
//    glLoadIdentity();
//    glBegin(GL_QUADS);
//    glVertex3f(-1.0f,-1.0f, -1.0f);
//    glVertex3f( 1.0f,-1.0f, -1.0f);
//    glVertex3f( 1.0f, 1.0f, -1.0f);
//    glVertex3f(-1.0f, 1.0f, -1.0f);
//    glEnd();
//    glPopMatrix();
//    glPopMatrix();

    glUseProgram( 0 );

    glPopAttrib();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneHandler::Render()
{
    unsigned int i;

	glClearColor( 0.2f, 0.2f, 0.4f, 0.0f );

	// Clear previous frame values
	glClear(GL_DEPTH_BUFFER_BIT);

    glCullFace(GL_BACK);

    handleResize();

    setupMatrices();

    deferred.DrawFirstPass();
    deferred.DrawSecondPass();

    SceneHandler::DrawLights();

//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//    glUseProgram(0);
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(-SceneHandler::width/4,SceneHandler::width/4,-SceneHandler::height/4,SceneHandler::height/4,1,20);
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
//    glColor4f(1.0,1.0,1.0,1.0);
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D,deferred.normalMap);
//    glEnable(GL_TEXTURE_2D);
//    glTranslated(0,0,-1);
//
//    glBegin(GL_QUADS);
//    glTexCoord2d(0,0);glVertex3f(0,0,0);
//    glTexCoord2d(1,0);glVertex3f(SceneHandler::width/4,0,0);
//    glTexCoord2d(1,1);glVertex3f(SceneHandler::width/4,SceneHandler::height/4,0);
//    glTexCoord2d(0,1);glVertex3f(0,SceneHandler::height/4,0);
//    glEnd();
//    glDisable(GL_TEXTURE_2D);
}

void SceneHandler::Destroy()
{
    unsigned int i;

    for(i=0; i< (SceneHandler::entity).size(); i++)
    {
        delete SceneHandler::entity.at(i);
    }
    for(i=0; i< SceneHandler::newtonEntity.size(); i++)
    {
        delete SceneHandler::newtonEntity.at(i);
    }
    for(i=0; i< SceneHandler::light.size(); i++)
    {
        delete SceneHandler::light.at(i);
    }

//#ifdef USE_VISUAL_DEBUGGER
//	// destroy the debugger server
//	NewtonDebuggerDestroyServer (g_newtonDebugger);
//#endif

	// destroy all rigid bodies, this is no necessary because Newton Destroy world will also destroy all bodies
	// but if you want to change level and restart you can call this function to clean the world without destroying the world.
	NewtonDestroyAllBodies (SceneHandler::world);

	// finally destroy the newton world
	NewtonDestroy (SceneHandler::world);

}

Entity* SceneHandler::CreateEntity()
{
    Entity* ent = new Entity();
    SceneHandler::entity.push_back(ent);

    return ent;
}

PlayerEntity* SceneHandler::CreatePlayerEntity()
{
    PlayerEntity* ent = new PlayerEntity();
    SceneHandler::newtonEntity.push_back((NewtonEntity*)ent);

    return ent;
}

MD5Model* SceneHandler::CreateMD5Entity(std::string modelPath, std::string animPath)
{
    MD5Model* model = new MD5Model();
    model->LoadModel(modelPath);
    model->LoadAnim(animPath);
    SceneHandler::entity.push_back(model);
    SceneHandler::renderList.push_back(model);
    return model;
}

ParticleSystemEntity* SceneHandler::CreateParticleSystem()
{
		ParticleSystemEntity* ent = new ParticleSystemEntity();
		SceneHandler::entity.push_back(ent);
    SceneHandler::renderList.push_back(ent);

    return ent;
}

StaticEntity* SceneHandler::CreateStaticEntity(string s, float scale)
{
    StaticEntity* ent = new StaticEntity();
    ent->scale=scale;
    ent->meshObj = new MeshObject();
    LoadAse(s,*ent->meshObj);
    ent->CalculateBounds();
    cout<<"SCALE: "<<scale<<endl;
    SceneHandler::newtonEntity.push_back(ent);

    //renderList should be filled before each frame with entities whos boundingboxes are inside the view-volume.
    SceneHandler::renderList.push_back(ent);

    return ent;
}

void SceneHandler::CreateBBoxCollision(NewtonEntity* ent, float mass)
{
    NewtonCollision* shape;
    NewtonBody* body;

    shape = CreateNewtonBox (SceneHandler::world, ent, 0);
	body = CreateRigidBody (SceneHandler::world, ent, shape, mass);

	NewtonReleaseCollision (SceneHandler::world, shape);
}

void SceneHandler::CreateConvexCollision(StaticEntity* ent, float mass)
{
    NewtonCollision* shape;
    NewtonBody* body;

    shape = CreateNewtonConvex (SceneHandler::world, ent, 0);
	body = CreateRigidBody (SceneHandler::world, ent, shape, mass);

	NewtonReleaseCollision (SceneHandler::world, shape);
}

void SceneHandler::CreateMeshCollision(StaticEntity* ent, float mass)
{
    NewtonCollision* shape;
    NewtonBody* body;

    shape = CreateNewtonMesh(SceneHandler::world, ent, 0);
	body = CreateRigidBody (SceneHandler::world, ent, shape, mass);

	NewtonReleaseCollision (SceneHandler::world, shape);
}

void SceneHandler::CreatePlayerCollision(PlayerEntity* ent)
{

    NewtonBody* body;
    NewtonCollision* shape;

    // calculate player high and width
    float padding = 1.0f / 64.0f;  // this is the default padding, for the palyer joint, we must subtract it from the shape
    float playerHigh = (ent->maxBox.z - ent->minBox.z) - padding;
    float playerRadius0 = (ent->maxBox.y - ent->minBox.y) * 0.5f;
    float playerRadius1 = (ent->maxBox.x - ent->minBox.x) * 0.5f;
    float playerRadius = (playerRadius0 > playerRadius1 ? playerRadius0 : playerRadius1) - padding;


    glm::mat4 orientation;
    //glm::gtc::matrix_access::column(orientation,0,glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
    orientation[0] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    orientation[1] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    orientation[2] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    orientation[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // add a body with a box shape
    //shape = CreateNewtonCapsule (SceneHandler::world, ent, playerHigh, playerRadius, 0, orientation);
    shape = CreateNewtonCylinder (SceneHandler::world, ent, playerHigh, playerRadius, 0, orientation);
    //shape = CreateNewtonBox (SceneHandler::world, ent, 0);
    body = CreateRigidBody (SceneHandler::world, ent, shape, 0);
    NewtonReleaseCollision (SceneHandler::world, shape);

    NewtonBodySetAutoSleep (body, 0);

    ent->playerBody = body;
}

void SceneHandler::SetInterpolationParam(float t)
{
    if (t > 1.0f) {
		t = 1.0f;
	}
    interpolationParam = t;
}

// this is the call back for allocation newton memory
void* AllocMemory (int sizeInBytes)
{
	return malloc (sizeInBytes);
}

// this is the callback for freeing Newton Memory
void FreeMemory (void *ptr, int sizeInBytes)
{
	free (ptr);
}

void ProcessEvents(NewtonWorld* world)
{
}

void AdvanceSimulation (int timeInMicroseconds)
{
	int deltaTime;

	// get the time step
	deltaTime = timeInMicroseconds - g_currentTime;
	g_currentTime = timeInMicroseconds;
	g_timeAccumulator += deltaTime;

	// advance the simulation at a fix step
	int loops = 0;

	while ((loops < MAX_PHYSICS_LOOPS) && (g_timeAccumulator >= FPS_IN_MICROSECONDS))
	{
		loops ++;

		// Process incoming events.
		ProcessEvents (SceneHandler::world);

		// run the newton update function
		NewtonUpdate (SceneHandler::world, std::max(SceneHandler::g_dt,(1.0f / PHYSICS_FPS)));

        PlayerEntity::NewtonUpdate(std::max(SceneHandler::g_dt,(1.0f / PHYSICS_FPS)));

		// subtract time from time accumulator
		g_timeAccumulator -= FPS_IN_MICROSECONDS;

	}

	if (loops > MAX_PHYSICS_LOOPS)
		g_timeAccumulator = FPS_IN_MICROSECONDS;

	// calculate the interpolation parameter for smooth rendering
	SceneHandler::SetInterpolationParam(float (g_timeAccumulator) / float(FPS_IN_MICROSECONDS));

}

Light* SceneHandler::CreateLight()
{
    Light *l = new Light();
    SceneHandler::light.push_back(l);

    return l;
}

Light* SceneHandler::FindNearestLight(float x, float y, float z)
{
    Light* l=NULL;
    float dist2=1.0e10;
    float min=dist2;
    glm::vec3 pos(x,y,z);
    glm::vec3 v;

    for(unsigned int i=0; i<SceneHandler::light.size(); i++)
    {
        v=pos-SceneHandler::light.at(i)->getPosition3();
        dist2 = glm::dot(v,v);
        if ( dist2 < min )
        {
            min=dist2;
            l=SceneHandler::light.at(i);
        }
    }
    //return SceneHandler::light.at(0);
    return l;
}

