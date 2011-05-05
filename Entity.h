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
    virtual void Update(float interpolationParam, NewtonWorld* world){}
    virtual void Draw(){}
    virtual void DrawGeometry(){}
    virtual void CalculateBounds();

    //Vector3f position;
    //Vector3f rotation;

    //NEWTON
	// these are the element to represent the position and orientation state of a graphics object in the world
//	dMatrix m_matrix;					// current interpolated visual matrix
//	dVector m_curPosition;				// position one physics simulation step in the future
//	dVector m_prevPosition;             // position at the current physics simulation step
//	dQuaternion m_curRotation;          // rotation one physics simulation step in the future
//	dQuaternion m_prevRotation;         // rotation at the current physics simulation step
//	dVector m_minBox;                   // BoundingBox min
//	dVector m_maxBox;                   // BoundingBox max
//	dFloat m_radius;                    // Sphere containing object

    glm::mat4 matrix;
    glm::vec4 curPosition;
    glm::vec4 prevPosition;
    glm::quat curRotation;
    glm::quat prevRotation;
    glm::vec4 minBox;
    glm::vec4 maxBox;

	NewtonBody* body;

    std::string name;
    bool visible;
    float scale;
private:

};

class PlayerEntity: public Entity
{
    public:

    static Camera* camera;
    static PlayerEntity* player;
    static bool followMouse;
    static void ApplyPlayerInput (const NewtonUserJoint* me, float timestep, int threadIndex);
    static void SetTransform (const NewtonBody* body, const float* matrix, int threadId);
    static void SetCamera(Camera* cam) {PlayerEntity::camera = cam;}
    //static void BodyIterator (const NewtonBody* body, void* userData);

    NewtonUserJoint* playerController;
    NewtonBody* playerBody;
    float eyeHeight;

    PlayerEntity();
    //void (*m_setTransformOriginal) (const NewtonBody* body, const dFloat* matrix, int threadIndex);
    void Update(float interpolationParam, NewtonWorld* world);
    void Draw();

    void UpdateCollision(NewtonBody* body);

    void UpdatePhysics(NewtonWorld* world);

    private:
    bool airborne;
    glm::vec3 velocity;
};

class MeshEntity: public Entity
{
public:
    MeshEntity();
    ~MeshEntity();
    void Scale(float s);
    unsigned int totalVertices;
    std::vector<Mesh*> mesh;
    std::vector<Material*> material;
};

class StaticEntity: public MeshEntity
{
public:
    void Update(float interpolationParam, NewtonWorld* world);
    void Draw();
    void DrawGeometry();
    void CalculateBounds();
};

class AnimatedEntity: public MeshEntity
{
public:
    void Draw();
};

#endif
