#ifndef SceneHandler_H
#define SceneHandler_H

#include <vector>
#include "Entity.h"
#include "Light.h"
#include "dMath\dMathDefines.h"

class SceneHandler
{
public:
    static int width, height;
    static NewtonWorld* world;

    static int Init();
    static void Destroy();

    static Entity* CreateEntity();
    static PlayerEntity* CreatePlayerEntity();
    static StaticEntity* CreateStaticEntity(string s, float scale=1.0f);

    static Light* CreateLight();

    static void SetInterpolationParam(dFloat t);
    static dFloat GetInterpolationParam(){ return interpolationParam; }

    static void CreateBBoxCollision(Entity* ent, float mass=0.0f);
    static void CreateConvexCollision(MeshEntity* ent, float mass=0.0f);
    static void CreateMeshCollision(MeshEntity* ent, float mass=0.0f);
    static void CreatePlayerCollision(PlayerEntity* ent, float mass=1.0f);
    static void CreatePlayCollision(PlayerEntity* ent);

    static Light* FindNearestLight(float x, float y, float z);

    static void Render();
    static void Update();

    static dFloat g_dt;
private:
    SceneHandler() {};                           // Private constructor
    SceneHandler(const SceneHandler&);                 // Prevent copy-construction
    SceneHandler& operator=(const SceneHandler&);      // Prevent assignment

    static void GenerateShadowMaps();

    static dFloat interpolationParam;



    static vector<Entity*> renderList;

    static vector<Entity*> entity;
    static vector<MeshEntity*> meshEntity;

    static vector<Light*> light;
};

#endif
