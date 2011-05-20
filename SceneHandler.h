#ifndef SceneHandler_H
#define SceneHandler_H

#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glfw.h>

#include <vector>
#include "Entity.h"
#include "Light.h"
#include "ResourceManager.h"

class SceneHandler
{
public:
    static int width, height;
    static NewtonWorld* world;

    static int Init();
    static void Destroy();

    static Entity* CreateEntity();
    static PlayerEntity* CreatePlayerEntity();
    static StaticEntity* CreateStaticEntity(std::string s, float scale=1.0f);

    static Light* CreateLight();

    static void SetInterpolationParam(float t);
    static float GetInterpolationParam(){ return interpolationParam; }

    static void CreateBBoxCollision(NewtonEntity* ent, float mass=0.0f);
    static void CreateConvexCollision(StaticEntity* ent, float mass=0.0f);
    static void CreateMeshCollision(StaticEntity* ent, float mass=0.0f);
    static void CreatePlayerCollision(PlayerEntity* ent);

    static Light* FindNearestLight(float x, float y, float z);

    static void Render();
    static void Update();

    static ResourceManager resources;

    static GLuint lightMap;
    static GLuint lightMapFBO;
    static GLuint lightMapDepth;
    static GLuint shadowShader;

    static void InitLightMap();

    static float g_dt;
private:
    SceneHandler() {};                                  // Private constructor
    SceneHandler(const SceneHandler&);                 // Prevent copy-construction
    SceneHandler& operator=(const SceneHandler&);      // Prevent assignment

    static void GenerateShadowMaps();

    static void DrawLightMap();

    static float interpolationParam;

    static std::vector<Entity*> renderList;

    static std::vector<Entity*> entity;
    static std::vector<NewtonEntity*> newtonEntity;

    static std::vector<Light*> light;
};

#endif
