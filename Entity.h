#ifndef Entity_H
#define Entity_H

#include <vector>
#include "Mesh.h"
#include "Material.h"

using std::vector;

class Entity
{
public:
    Entity();
    ~Entity();
    void Remove();
    void AddChild(Entity &obj);
    void AddChild(Entity *obj);
    void SetPosition(float px, float py, float pz);
    void SetRotation(float rx, float ry, float rz);
    void SetVisibility(bool b);
    void SetName(char *c);
    int NumChildren();
    virtual void Draw();

    Vector3f position;
    Vector3f rotation;

    int parentIndex;

    char *name;
    bool visible;
    int numChildren;
    vector<Entity*> child;
    vector<Entity*>::const_iterator cit;
    Entity *parent;
private:
};

class SphereEntity: public Entity
{
public:
    SphereEntity(float r=1.0f) : Entity() , radius(r) {};
    void Draw();
    void SetRadius(float r);
    float radius;
};

class MeshEntity: public Entity
{
public:
    Mesh *mesh;
    Material *material;
};

class StaticEntity: public MeshEntity
{
public:
    void Draw();
};

class AnimatedEntity: public MeshEntity
{
public:
    void Draw();
};

#endif
