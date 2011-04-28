#ifndef Physics_H
#define Physics_H

#include <Newton.h>
#include "Entity.h"

NewtonCollision* CreateNewtonBox (NewtonWorld* world, Entity *ent, int shapeId);

NewtonCollision* CreateNewtonConvex (NewtonWorld* world, MeshEntity *ent, int shapeId);

NewtonCollision* CreateNewtonMesh (NewtonWorld* world, MeshEntity* ent, int* shapeIdArray);

NewtonCollision* CreateNewtonCylinder (NewtonWorld* world, Entity *ent, dFloat height, dFloat radius, int shapeId, const dMatrix& orientation);

NewtonCollision* CreateNewtonCapsule (NewtonWorld* world, Entity *ent, dFloat height, dFloat radius, int shapeId, const dMatrix& orientation);

NewtonBody* CreateRigidBody (NewtonWorld* world, Entity* ent, NewtonCollision* collision, dFloat mass);

void DestroyBodyCallback (const NewtonBody* body);

// Transform callback to set the matrix of a the visual entity
void SetTransformCallback (const NewtonBody* body, const dFloat* matrix, int threadIndex);

// callback to apply external forces to body
void ApplyForceAndTorqueCallback (const NewtonBody* body, dFloat timestep, int threadIndex);

#endif
