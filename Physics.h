#ifndef Physics_H
#define Physics_H

#include <Newton.h>
#include "Entity.h"
#include "Math.h"

NewtonCollision* CreateNewtonBox (NewtonWorld* world, NewtonEntity *ent, int shapeId);

NewtonCollision* CreateNewtonConvex (NewtonWorld* world, StaticEntity *ent, int shapeId);

NewtonCollision* CreateNewtonMesh (NewtonWorld* world, StaticEntity* ent, int* shapeIdArray);

NewtonCollision* CreateNewtonCylinder (NewtonWorld* world, NewtonEntity *ent, float height, float radius, int shapeId, const glm::mat4& orientation);

NewtonCollision* CreateNewtonCapsule (NewtonWorld* world, NewtonEntity *ent, float height, float radius, int shapeId, const glm::mat4& orientation);

NewtonBody* CreateRigidBody (NewtonWorld* world, NewtonEntity* ent, NewtonCollision* collision, float mass);

void DestroyBodyCallback (const NewtonBody* body);

// Transform callback to set the matrix of a the visual entity
void SetTransformCallback (const NewtonBody* body, const float* matrix, int threadIndex);

// callback to apply external forces to body
void ApplyForceAndTorqueCallback (const NewtonBody* body, float timestep, int threadIndex);

#endif
