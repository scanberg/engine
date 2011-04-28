#include "Physics.h"
#include <GL/glfw.h>

NewtonCollision* CreateNewtonBox (NewtonWorld* world, Entity *ent, int shapeId)
{
	dVector minBox = ent->m_minBox;
	dVector maxBox = ent->m_maxBox;
	NewtonCollision* collision;

	// Get the Bounding Box for this entity
	//ent->GetBBox (minBox, maxBox);

	//calculate the box size and dimensions of the physics collision shape
	dVector size (maxBox - minBox);
	dVector origin ((maxBox + minBox).Scale (0.5f));
	size.m_w = 1.0f;
	origin.m_w = 1.0f;

	// make and offset Matrix for this collision shape.
	dMatrix offset (GetIdentityMatrix());
	offset.m_posit = origin;

	// now create a collision Box for this entity
	collision = NewtonCreateBox (world, size.m_x, size.m_y, size.m_z, shapeId, &offset[0][0]);

	return collision;
}

NewtonCollision* CreateNewtonConvex (NewtonWorld* world, MeshEntity *ent, int shapeId)
{
	dVector minBox = ent->m_minBox;
	dVector maxBox = ent->m_maxBox;
	NewtonCollision* collision;
	dVector* tmpArray = new dVector [ent->totalVertices];

	// Get the Bounding Box for this entity
	//ent->GetBBox (minBox, maxBox);

	dVector size (maxBox - minBox);
	dVector origin ((maxBox + minBox).Scale (0.5f));
	size.m_w = 1.0f;
	origin.m_w = 1.0f;

	Vector3f v_origin = Vector3f( (float)origin.m_x , (float)origin.m_y, (float)origin.m_z);

	unsigned int i,u,counter;
	counter=0;

    // Translate al the points to the origin point
    for(i=0; i<ent->mesh.size(); i++)
    {
        for(u=0; u<ent->mesh.at(i)->numVertices; u++)
        {

            dVector tmp (ent->mesh[i]->vertex[u].x, ent->mesh[i]->vertex[u].y, ent->mesh[i]->vertex[u].z, 0.0f);
            tmpArray[counter] = tmp - origin;
            counter++;
        }
    }

	// make and offset Matrix for this collision shape.
	dMatrix offset (GetIdentityMatrix());
	offset.m_posit = origin;

	// now create a convex hull shape from the vertex geometry
	collision = NewtonCreateConvexHull(world, ent->totalVertices, &tmpArray[0][0], sizeof (dVector), 0.1f, shapeId, &offset[0][0]);

	delete tmpArray;

	return collision;
}

NewtonCollision* CreateNewtonMesh (NewtonWorld* world, MeshEntity* ent, int* shapeIdArray)
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
	for (i = 0; i <  ent->mesh.size(); i ++) {
		// add each sub mesh as a face id, will will use this later for a multi material sound effect in and advanced tutorial
		for (j = 0; j < ent->mesh[i]->numFaces; j++) {
			dVector face[3];

			vert=ent->mesh[i]->vertex[ent->mesh[i]->face[j].point[0]];
            face[0] = dVector (vert.x, vert.y, vert.z);

            vert=ent->mesh[i]->vertex[ent->mesh[i]->face[j].point[1]];
            face[1] = dVector (vert.x, vert.y, vert.z);

            vert=ent->mesh[i]->vertex[ent->mesh[i]->face[j].point[2]];
            face[2] = dVector (vert.x, vert.y, vert.z);

            counter++;

			if (shapeIdArray) {
				NewtonTreeCollisionAddFace(collision, 3, &face[0].m_x, sizeof (dVector), shapeIdArray[i]);
			} else {
				NewtonTreeCollisionAddFace(collision, 3, &face[0].m_x, sizeof (dVector), i + 1);
			}


		}
	}

	// end adding faces to the collision tree, also optimize the mesh for best performance
	NewtonTreeCollisionEndBuild (collision, 1);

	return collision;
}

NewtonCollision* CreateNewtonCylinder (NewtonWorld* world, Entity *ent, dFloat height, dFloat radius, int shapeId, const dMatrix& orientation)
{
	dVector minBox = ent->m_minBox;
	dVector maxBox = ent->m_maxBox;

	// Get the Bounding Box for this entity
	//ent->GetBBox (minBox, maxBox);

	dMatrix offset (orientation);
	// Place the shape origin at the geometrical center of the entity
	offset.m_posit = (maxBox + minBox).Scale (0.5f);
	offset.m_posit.m_w = 1.0f;

	// now create a collision Box for this entity
	return NewtonCreateCylinder(world, radius, height, shapeId, &offset[0][0]);
}

NewtonCollision* CreateNewtonCapsule (NewtonWorld* world, Entity *ent, dFloat height, dFloat radius, int shapeId, const dMatrix& orientation)
{
	dVector minBox = ent->m_minBox;
	dVector maxBox = ent->m_maxBox;

	// Get the Bounding Box for this entity
	//ent->GetBBox (minBox, maxBox);

	dMatrix offset (orientation);
	// Place the shape origin at the geometrical center of the entity
	offset.m_posit = (maxBox + minBox).Scale (0.5f);
	offset.m_posit.m_w = 1.0f;

	// now create a collision Box for this entity
	return NewtonCreateCapsule(world, radius, height, shapeId, &offset[0][0]);
}

NewtonBody* CreateRigidBody (NewtonWorld* world, Entity* ent, NewtonCollision* collision, dFloat mass)
{
	dVector minBox;
	dVector maxBox;
	dVector origin;
	dVector inertia;
	NewtonBody* body;

	// we need to set physics properties to this body
	dMatrix matrix (ent->m_curRotation, ent->m_curPosition);
	//dMatrix matrix;
	//NewtonBodySetMatrix (body, &matrix[0][0]);


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
	NewtonBodySetMassMatrix (body, mass, mass * inertia.m_x, mass * inertia.m_y, mass * inertia.m_z);

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
void SetTransformCallback (const NewtonBody* body, const dFloat* matrix, int threadIndex)
{
	Entity* ent;

	// Get the position from the matrix
	dVector posit (matrix[12], matrix[13], matrix[14], 1.0f);
	dQuaternion rotation;

	// we will ignore the Rotation part of matrix and use the quaternion rotation stored in the body
	NewtonBodyGetRotation(body, &rotation.m_q0);

	// get the entity associated with this rigid body
	ent = (Entity*) NewtonBodyGetUserData(body);

	// since this tutorial run the physics and a different fps than the Graphics
	// we need to save the entity current transformation state before updating the new state.
	ent->m_prevPosition = ent->m_curPosition;
	ent->m_prevRotation = ent->m_curRotation;

	if (ent->m_curRotation.DotProduct (rotation) < 0.0f) {
		ent->m_prevRotation.Scale(-1.0f);
	}

	// set the new position and orientation for this entity
	ent->m_curPosition = posit;
	ent->m_curRotation = rotation;
}


// callback to apply external forces to body
void ApplyForceAndTorqueCallback (const NewtonBody* body, dFloat timestep, int threadIndex)
{
	dFloat Ixx;
	dFloat Iyy;
	dFloat Izz;
	dFloat mass;

	// for this tutorial the only external force in the Gravity
	NewtonBodyGetMassMatrix (body, &mass, &Ixx, &Iyy, &Izz);

	dFloat y=0.0;
	dFloat x=0.0;

    if(glfwGetKey(GLFW_KEY_UP))
        y=1.0;
    if(glfwGetKey(GLFW_KEY_DOWN))
        y=-1.0;
    if(glfwGetKey(GLFW_KEY_LEFT))
        x=-1.0;
    if(glfwGetKey(GLFW_KEY_RIGHT))
        x=1.0;
	dVector gravityForce  (300.0f*x*mass, 300.0f*y*mass, mass * -100.0f, 1.0f);
	NewtonBodySetForce(body, &gravityForce[0]);
}
