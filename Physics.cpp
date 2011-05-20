#include "Physics.h"
#include <GL/glfw.h>


NewtonCollision* CreateNewtonBox (NewtonWorld* world, NewtonEntity *ent, int shapeId)
{
	NewtonCollision* collision;

	// Get the Bounding Box for this entity
	//ent->GetBBox (minBox, maxBox);

	//calculate the box size and dimensions of the physics collision shape
	glm::vec3 size( ent->maxBox - ent->minBox );
	glm::vec3 origin( (ent->maxBox + ent->minBox)*0.5f );


	// make and offset Matrix for this collision shape.
//	dMatrix offset (GetIdentityMatrix());
//	offset.m_posit = origin;
	glm::mat4 offset = glm::gtc::matrix_transform::translate(glm::mat4(1.0f),origin);

	// now create a collision Box for this entity
	collision = NewtonCreateBox (world, size.x, size.y, size.z, shapeId, &offset[0][0]);

	return collision;
}

NewtonCollision* CreateNewtonConvex (NewtonWorld* world, StaticEntity *ent, int shapeId)
{
	NewtonCollision* collision;
	glm::vec3* tmpArray = new glm::vec3 [ent->meshObj->totalVertices];

	glm::vec3 size (ent->maxBox - ent->minBox);
	glm::vec3 origin ((ent->maxBox + ent->minBox)*0.5f);

	unsigned int i,u,counter;
	counter=0;

    // Translate all the points to the origin point
    for(i=0; i<ent->meshObj->mesh.size(); i++)
    {
        for(u=0; u<ent->meshObj->mesh.at(i)->numVertices; u++)
        {

            glm::vec3 tmp(ent->meshObj->mesh[i]->vertex[u].x, ent->meshObj->mesh[i]->vertex[u].y, ent->meshObj->mesh[i]->vertex[u].z);
            tmpArray[counter] = tmp - origin;
            counter++;
        }
    }

	// make and offset Matrix for this collision shape.
	glm::mat4 offset = glm::gtc::matrix_transform::translate(glm::mat4(1.0f),origin);

	// now create a convex hull shape from the vertex geometry
	collision = NewtonCreateConvexHull(world, ent->meshObj->totalVertices, &tmpArray[0][0], sizeof (glm::vec3), 0.1f, shapeId, &offset[0][0]);

	delete tmpArray;

	return collision;
}

NewtonCollision* CreateNewtonMesh (NewtonWorld* world, StaticEntity* ent, int* shapeIdArray)
{
	NewtonCollision* collision;

	// now create and empty collision tree
	collision = NewtonCreateTreeCollision (world, 0);

	// start adding faces to the collision tree
	NewtonTreeCollisionBeginBuild (collision);

	Vertex vert;
	unsigned int i,j;
	unsigned int counter = 0;

	// step over the collision geometry and add all faces to the collision tree
	for (i = 0; i <  ent->meshObj->mesh.size(); i ++) {
		// add each sub mesh as a face id, will will use this later for a multi material sound effect in and advanced tutorial
		for (j = 0; j < ent->meshObj->mesh[i]->numFaces; j++) {
			glm::vec3 face[3];

			vert=ent->meshObj->mesh[i]->vertex[ent->meshObj->mesh[i]->face[j].point[0]];
            face[0] = glm::vec3(vert.x, vert.y, vert.z)*ent->scale;

            vert=ent->meshObj->mesh[i]->vertex[ent->meshObj->mesh[i]->face[j].point[1]];
            face[1] = glm::vec3 (vert.x, vert.y, vert.z)*ent->scale;

            vert=ent->meshObj->mesh[i]->vertex[ent->meshObj->mesh[i]->face[j].point[2]];
            face[2] = glm::vec3 (vert.x, vert.y, vert.z)*ent->scale;

            counter++;

			if (shapeIdArray) {
				NewtonTreeCollisionAddFace(collision, 3, &face[0].x, sizeof (glm::vec3), shapeIdArray[i]);
			} else {
				NewtonTreeCollisionAddFace(collision, 3, &face[0].x, sizeof (glm::vec3), i + 1);
			}


		}
	}

	// end adding faces to the collision tree, also optimize the mesh for best performance
	NewtonTreeCollisionEndBuild (collision, 1);

	return collision;
}

NewtonCollision* CreateNewtonCylinder (NewtonWorld* world, NewtonEntity *ent, float height, float radius, int shapeId, const glm::mat4& orientation)
{
	// Place the shape origin at the geometrical center of the entity
	glm::vec3 origin( (ent->maxBox + ent->minBox)*0.5f );

    glm::mat4 offset = glm::gtc::matrix_transform::translate(orientation,origin);

	// now create a collision Box for this entity
	return NewtonCreateCylinder(world, radius, height, shapeId, &offset[0][0]);
}

NewtonCollision* CreateNewtonCapsule (NewtonWorld* world, NewtonEntity *ent, float height, float radius, int shapeId, const glm::mat4& orientation)
{
	// Place the shape origin at the geometrical center of the entity
	glm::vec3 origin( (ent->maxBox + ent->minBox)*0.5f );

    glm::mat4 offset = glm::gtc::matrix_transform::translate(orientation,origin);

	// now create a collision Box for this entity
	return NewtonCreateCapsule(world, radius, height, shapeId, &offset[0][0]);
}

NewtonBody* CreateRigidBody (NewtonWorld* world, NewtonEntity* ent, NewtonCollision* collision, dFloat mass)
{
	glm::vec3 minBox;
	glm::vec3 maxBox;
	glm::vec3 origin;
	glm::vec3 inertia;
	NewtonBody* body;

    glm::mat4 matrix = createMat4(ent->curRotation,ent->curPosition);

	// Now with the collision Shape we can crate a rigid body
	body = NewtonCreateBody (world, collision, &matrix[0][0]);

	// bodies can have a destructor.
	// this is a function callback that can be used to destroy any local data stored
	// and that need to be destroyed before the body is destroyed.
	NewtonBodySetDestructorCallback (body, DestroyBodyCallback);

	// save the entity as the user data for this body
	NewtonBodySetUserData (body, ent);
	ent->body=body;

	// we need to set the proper center of mass and inertia matrix for this body
	// the inertia matrix calculated by this function does not include the mass.
	// therefore it needs to be multiplied by the mass of the body before it is used.
	NewtonConvexCollisionCalculateInertialMatrix (collision, &inertia[0], &origin[0]);

	// set the body mass matrix
	NewtonBodySetMassMatrix (body, mass, mass * inertia.x, mass * inertia.y, mass * inertia.z);

	// set the body origin
	NewtonBodySetCentreOfMass (body, &origin[0]);

	// set the function callback to apply the external forces and torque to the body
	// the most common force is Gravity
	NewtonBodySetForceAndTorqueCallback (body, ApplyForceAndTorqueCallback);

	// set the function callback to set the transformation state of the graphic entity associated with this body
	// each time the body change position and orientation in the physics world
	NewtonBodySetTransformCallback (body, SetTransformCallback);

	return body;
}

void DestroyBodyCallback (const NewtonBody* body)
{
	// for now there is nothing to destroy
}

// Transform callback to set the matrix of a the visual entity
void SetTransformCallback (const NewtonBody* body, const float* matrix, int threadIndex)
{
	NewtonEntity* ent;

	// Get the position from the matrix
	glm::vec4 position (matrix[12], matrix[13], matrix[14], 1.0f);
	glm::quat rotation;

	// we will ignore the Rotation part of matrix and use the quaternion rotation stored in the body
	NewtonBodyGetRotation(body, &rotation[0]);

	// get the entity associated with this rigid body
	ent = (NewtonEntity*) NewtonBodyGetUserData(body);

	// since this tutorial run the physics and a different fps than the Graphics
	// we need to save the entity current transformation state before updating the new state.
	ent->prevPosition = ent->curPosition;
	ent->prevRotation = ent->curRotation;

	if (glm::dot(ent->curRotation,rotation) < 0.0f) {
		ent->prevRotation *= -1.0f;
	}

	// set the new position and orientation for this entity
	ent->curPosition = position;
	ent->curRotation = rotation;
}


// callback to apply external forces to body
void ApplyForceAndTorqueCallback (const NewtonBody* body, float timestep, int threadIndex)
{
	float Ixx;
	float Iyy;
	float Izz;
	float mass;

	// for this tutorial the only external force in the Gravity
	NewtonBodyGetMassMatrix (body, &mass, &Ixx, &Iyy, &Izz);

	float y=0.0;
	float x=0.0;

    if(glfwGetKey(GLFW_KEY_UP))
        y=1.0;
    if(glfwGetKey(GLFW_KEY_DOWN))
        y=-1.0;
    if(glfwGetKey(GLFW_KEY_LEFT))
        x=-1.0;
    if(glfwGetKey(GLFW_KEY_RIGHT))
        x=1.0;
	glm::vec4 gravityForce  (300.0f*x*mass, 300.0f*y*mass, mass * -100.0f, 1.0f);
	NewtonBodySetForce(body, &gravityForce[0]);
}
