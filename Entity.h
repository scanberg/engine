#ifndef Entity_H
#define Entity_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>
#include <vector>
#include <Newton.h>
#include <JointLibrary.h>
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"
#include "Math.h"
#include "Material.h"

class Entity
{
public:
    Entity();
    ~Entity();
    void Remove();
    void SetPosition(float px, float py, float pz);
    void SetRotation(float rx, float ry, float rz);
    void SetVisibility(bool b);
    void SetName(const std::string& s);
    virtual void SetScale(float s){scale=s;}
    virtual void Update(){}
    virtual void Draw(){}
    virtual void DrawShadow(){}
    virtual void DrawFirstPass(GLuint shader){}
    virtual void DrawGeometry(){}
    virtual void CalculateBounds();

    //Bounding box
    glm::vec4 minBox;
    glm::vec4 maxBox;

    //local->world matrix
    glm::mat4 matrix;

    std::string name;
    bool visible;
    float scale;
private:

};

class NewtonEntity: public Entity
{
public:
    NewtonEntity();
    void SetPosition(float px, float py, float pz);
    void SetRotation(float rx, float ry, float rz);
    virtual void Update(float interpolationParam, NewtonWorld* world);

    glm::vec4 curPosition;
    glm::vec4 prevPosition;
    glm::quat curRotation;
    glm::quat prevRotation;

	NewtonBody* body;
private:
};

class PlayerEntity: public NewtonEntity
{
    public:

    static Camera* camera;
    static PlayerEntity* player;
    static bool followMouse;
    static void ApplyPlayerInput (const NewtonUserJoint* me, float timestep, int threadIndex);
    static void NewtonUpdate (float dt);
    static void SetCamera(Camera* cam) {PlayerEntity::camera = cam;}

    NewtonUserJoint* playerController;
    NewtonBody* playerBody;
    float eyeHeight;

    PlayerEntity();

    void Update(float interpolationParam, NewtonWorld* world);

    void UpdateCollision(NewtonBody* body);
    void UpdatePhysics(NewtonWorld* world, float dt);

    private:
    bool airborne;
    glm::vec3 velocity;
};

class MeshObject
{
public:
    MeshObject();
    ~MeshObject();
    void Scale(float s);
    unsigned int totalVertices;
    std::vector<Mesh*> mesh;
    std::vector<Material*> material;
};

class StaticEntity: public NewtonEntity
{
public:
    void Draw();
    void DrawShadow();
    void DrawFirstPass(GLuint shader);
    void DrawGeometry();
    void CalculateBounds();
    MeshObject* meshObj;
private:

};

#endif
