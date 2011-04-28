#define GLEW_STATIC
#include <GL/glew.h>

#include <GL/glfw.h>
#include <math.h>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <set>
#include "Entity.h"
#include "Mesh.h"
#include "Material.h"
#include "SceneHandler.h"

using namespace std;

Camera* PlayerEntity::camera = NULL;
bool PlayerEntity::followMouse = false;
static dVector minPush;
static dVector maxPush;

Entity::Entity() :
	m_matrix (GetIdentityMatrix()),
	m_curPosition (0.0f, 0.0f, 0.0f, 1.0f),
	m_prevPosition (0.0f, 0.0f, 0.0f, 1.0f),
	m_curRotation (1.0f, 0.0f, 0.0f, 0.0f),
	m_prevRotation (1.0f, 0.0f, 0.0f, 0.0f)
{
    position.x=position.y=position.z=0.0;
    rotation=position;
    visible=true;
    scale=1.0;
}

Entity::~Entity() {};

void Entity::Remove()
{
    delete this;
};

void Entity::SetPosition(float px, float py, float pz)
{
    position.x=px;
    position.y=py;
    position.z=pz;
    m_curPosition=m_prevPosition=m_matrix.m_posit=dVector(px,py,pz);
}
void Entity::SetRotation(float rx, float ry, float rz)
{
    rotation.x=rx;
    rotation.y=ry;
    rotation.z=rz;
}

void Entity::SetVisibility(bool b)
{
    visible=b;
}
void Entity::SetName(const string& s)
{
    name = s;
}

void Entity::CalculateBounds()
{

}

void PlayerEntity::Draw()
{
}

dFloat Inc(dFloat target, dFloat current, dFloat stepsize=1.0)
{
    if (current<target)
    {
        current += stepsize;
        current = min(current,target);
    }
    else if(current>target)
    {
        current -= stepsize;
        current = max(current,target);
    }
    return current;
}

int NewtonBodyCollide(NewtonWorld* world, int maxsize, NewtonBody* body0, NewtonBody* body1, dFloat* contacts, dFloat* normals, dFloat* penetration)
{
      NewtonCollision* collision[2];
      dFloat mat0[16];
      dFloat mat1[16];
      collision[0]=NewtonBodyGetCollision(body0);
      collision[1]=NewtonBodyGetCollision(body1);
      NewtonBodyGetMatrix(body0,mat0);
      NewtonBodyGetMatrix(body1,mat1);
      return NewtonCollisionCollide(world,maxsize,collision[0],mat0,collision[1],mat1,contacts,normals,penetration,0);
}

void PlayerEntity::UpdateCollision(NewtonBody* body)
{
    const int maxSize = 64;
    int numContacts;
    dFloat contacts[3*maxSize];
    dFloat normals[3*maxSize];
    dFloat depths[maxSize];
    dVector pos = m_matrix.m_posit;
    dVector contVec;
    dVector normal;

    numContacts=NewtonBodyCollide(SceneHandler::world,maxSize,this->body,body,&contacts[0],&normals[0],&depths[0]);

    for(int i=0; i<numContacts; i++)
    {
//        printf("col! pos: (%.1f,%.1f,%.1f) norm: (%.1f,%.1f,%.1f) depth: %.1f \n",
//                contacts[3*i+0],contacts[3*i+1],contacts[3*i+2],
//                normals[3*i+0],normals[3*i+1],normals[3*i+2],depths[i] );

        if(depths[i] > abs(0.0f))
        {

            contVec = dVector(contacts[3*i+0],contacts[3*i+1],contacts[3*i+2]) - pos;
            normal = dVector(normals[3*i+0],normals[3*i+1],normals[3*i+2]);

            //Se till att normalen pekar mot body
            if(contVec%normal>0.0f)
                normal=normal.Scale(-1.0);

            //normalize normal?
            //normal=normal.Scale(sqrt(normal%normal));
            //normal

            if(normal%dVector(0.0,0.0,1.0)>0.5)
                airborne=false;

            normal=normal.Scale(depths[i]);

            minPush.m_x = min(minPush.m_x,normal.m_x);
            minPush.m_y = min(minPush.m_y,normal.m_y);
            minPush.m_z = min(minPush.m_z,normal.m_z);
            maxPush.m_x = max(maxPush.m_x,normal.m_x);
            maxPush.m_y = max(maxPush.m_y,normal.m_y);
            maxPush.m_z = max(maxPush.m_z,normal.m_z);
        }

        //cout<<endl;
    }
    //bla bla

}

void BodyIterator (const NewtonBody* body, void* userData)
{
    //Undvik att hantera kollision med sig själv
    if(NewtonBodyGetUserData(body)!=userData)
    {
        PlayerEntity* player = (PlayerEntity*)userData;
        player->UpdateCollision((NewtonBody*)body);
    }
}

void PlayerEntity::Update(dFloat interpolationParam, NewtonWorld* world)
{
    static int mousex, mousey, lastmousex, lastmousey;
    bool followMouse=PlayerEntity::followMouse;

    Camera* cam=PlayerEntity::camera;

    lastmousex = mousex;
    lastmousey = mousey;

    glfwGetMousePos( &mousex, &mousey );

    if(followMouse)
    {
        cam->rot.z -= (lastmousex - mousex) * 0.2f;
        cam->rot.x -= (lastmousey - mousey) * 0.2f;
        if ( cam->rot.x < 179.5f ) cam->rot.x = 179.5f;
        if ( cam->rot.x > 359.0f ) cam->rot.x = 359.0f;
        if ( cam->rot.z > 360.0f ) cam->rot.z -= 360.0f;
        if ( cam->rot.z < -360.0f ) cam->rot.z += 360.0f;
    }

    // Calculate visual Transform by Interpolating between prev and curr State
	//dVector posit(m_prevPosition + (m_curPosition - m_prevPosition).Scale (interpolationParam));
//
	//m_prevPosition = m_curPosition;

	//m_matrix.m_posit = posit;

    dFloat move = (float)(glfwGetKey('W')-glfwGetKey('S'));
    dFloat strafe = (float)(glfwGetKey('D')-glfwGetKey('A'));

    dVector desiredVelocity(strafe,move,0.0);

    if(desiredVelocity%desiredVelocity > 1.0f)
    {
        //skala med 1/sqrt(2);
        desiredVelocity=desiredVelocity.Scale(0.70710678f);
    }

    //velocity.m_x=Inc(desiredVelocity.m_x,velocity.m_x,0.05f);
    //velocity.m_y=Inc(desiredVelocity.m_y,velocity.m_y,0.05f);

    dMatrix mat(0.0,0.0,-cam->rot.z*(3.1415926534f/180.0f),dVector(0.0,0.0,0.0));
    desiredVelocity=mat.RotateVector(desiredVelocity);

    velocity.m_x = desiredVelocity.m_x;
    velocity.m_y = desiredVelocity.m_y;

    if(!airborne&&glfwGetKey(GLFW_KEY_SPACE))
    {
        velocity.m_z=1.3;
    }

    velocity.m_z -= 0.022;
    airborne=true;

    if(velocity.m_z<=0.0)
    {
        //AlignToGroundConvex();
    }

    dVector prevPosition=m_matrix.m_posit;

    m_matrix.m_posit += velocity;

    NewtonBodySetMatrix(body,&m_matrix[0][0]);

    for(int i=0; i<3; i++)
    {
        dVector min = m_matrix.m_posit+m_minBox;
        dVector max = m_matrix.m_posit+m_maxBox;

        minPush.m_x=0.0f;
        minPush.m_y=0.0f;
        minPush.m_z=0.0f;
        maxPush.m_x=0.0f;
        maxPush.m_y=0.0f;
        maxPush.m_z=0.0f;

        NewtonWorldForEachBodyInAABBDo(world,&min[0],&max[0],BodyIterator,this);

        m_matrix.m_posit += (minPush+maxPush);

        NewtonBodySetMatrix(body,&m_matrix[0][0]);
    }

    velocity = m_matrix.m_posit - prevPosition;
    if(!airborne)
        velocity.m_z = min(velocity.m_z,0);

    cam->setPosition(m_matrix.m_posit.m_x,m_matrix.m_posit.m_y,m_matrix.m_posit.m_z);
}
//
void PlayerEntity::ApplyPlayerInput (const NewtonUserJoint* me, dFloat timestep, int threadIndex)
{
    dFloat velocity;
    dFloat strafeVeloc;
    dFloat headinAngle;

    static int mousex, mousey, lastmousex, lastmousey;
    bool followMouse=PlayerEntity::followMouse;

    Camera* cam=PlayerEntity::camera;

    lastmousex = mousex;
    lastmousey = mousey;

    glfwGetMousePos( &mousex, &mousey );

    if(followMouse)
    {
        cam->rot.z -= (lastmousex - mousex) * 0.2f;
        cam->rot.x -= (lastmousey - mousey) * 0.2f;
        if ( cam->rot.x < 179.5f ) cam->rot.x = 179.5f;
        if ( cam->rot.x > 359.0f ) cam->rot.x = 359.0f;
        if ( cam->rot.z > 360.0f ) cam->rot.z -= 360.0f;
        if ( cam->rot.z < -360.0f ) cam->rot.z += 360.0f;
    }

    const float PLAYER_SPEED = 100.0;

    velocity = 0.0f;
    if (glfwGetKey('W')) {
        velocity = PLAYER_SPEED;
    } else if (glfwGetKey  ('S')) {
        velocity = -PLAYER_SPEED;
    }

    strafeVeloc = 0.0f;
    if (glfwGetKey  ('D')) {
        strafeVeloc = PLAYER_SPEED;
    } else if (glfwGetKey  ('A')) {
        strafeVeloc = -PLAYER_SPEED;
    }

    // now set the desired player velocity and heading
    headinAngle = -(PlayerEntity::camera->rot.z) / 180.0 * 3.141592654f;//GetCameraYawAngle ();

    // prevent player fro running faster when strafing and moving forward as the same time
    dFloat mag2 = velocity * velocity + strafeVeloc * strafeVeloc;
    if (mag2 > PLAYER_SPEED * PLAYER_SPEED) {
        mag2 = PLAYER_SPEED * dSqrt (1.0f / mag2);
        velocity *= mag2;
        strafeVeloc *= mag2;
    }

    CustomPlayerControllerSetVelocity (me, velocity, strafeVeloc, headinAngle);
}

void PlayerEntity::SetTransform (const NewtonBody* body, const dFloat* matrix, int threadId)
{
    /*
    //m_setTransformOriginal(body,matrix,threadId);

    PlayerEntity* ent;

	// Get the position from the matrix
	dVector posit (matrix[12], matrix[13], matrix[14], 1.0f);

	// get the entity associated with this rigid body
	ent = (PlayerEntity*) NewtonBodyGetUserData(body);

	ent->m_prevPosition = ent->m_curPosition;

	// set the new position and orientation for this entity
	ent->m_curPosition = posit;

    dFloat dt = SceneHandler::GetInterpolationParam();

    // Calculate visual Transform by Interpolating between prev and curr State
	posit = dVector(ent->m_prevPosition + (ent->m_curPosition - ent->m_prevPosition).Scale (dt));

	PlayerEntity::camera->setPosition(posit.m_x,posit.m_y,posit.m_z);
	*/

	// get the entity associated with this rigid body
	PlayerEntity* ent = (PlayerEntity*) NewtonBodyGetUserData(body);

	ent->Update(0.0,NewtonBodyGetWorld(body));
}

MeshEntity::MeshEntity()
{
    totalVertices=0;
}

MeshEntity::~MeshEntity()
{
    unsigned int i;

    for(i=0;i<mesh.size();i++)
    {
        delete mesh.at(i);
    }

    // material innehåller inte unika pekare, därför skapas en set
    // där varje element är unikt och tas bort därefter.
    set<Material*> mat(material.begin(),material.end());
    set<Material*>::const_iterator sit;
    sit=mat.begin();

    while(sit != mat.end())
    {
        delete &sit;
        sit++;
    }
}

void MeshEntity::Scale(float s)
{
    unsigned int i;
    for(i=0; i<mesh.size(); i++)
    {
        mesh[i]->scale(s);
    }
}

void StaticEntity::Update(dFloat interpolationParam, NewtonWorld* world)
{
	// Calculate visual Transform by Interpolating between prev and curr State
	dVector posit (m_prevPosition + (m_curPosition - m_prevPosition).Scale (interpolationParam));
	dQuaternion rotation (m_prevRotation.Slerp(m_curRotation, interpolationParam));

	m_matrix = dMatrix (rotation, posit);
}

void StaticEntity::DrawGeometry()
{
    if(visible)
    {

        glPushMatrix();

        glMultMatrixf(&m_matrix[0][0]);

//        glMatrixMode(GL_TEXTURE);
//        glActiveTexture(GL_TEXTURE3);
//        glPushMatrix();
//        glMultMatrixf(&m_matrix[0][0]);

        for(unsigned int i=0; i<mesh.size(); i++)
        {
            if(mesh.at(i) != NULL)
            {
                mesh.at(i)->draw();
            }
        }


        //glPopMatrix();

        glPopMatrix();
    }
}

void StaticEntity::Draw()
{
    if(visible)
    {
        Light *nearestLight=SceneHandler::FindNearestLight(m_matrix.m_posit.m_x,
                                                           m_matrix.m_posit.m_y,
                                                           m_matrix.m_posit.m_z);
        if(nearestLight!=NULL)
        {
            //cout<<"inte null iaf"<<endl;
            nearestLight->assignTo(0);
        }

        glPushMatrix();
        glMultMatrixf(&m_matrix[0][0]);

//        glMatrixMode(GL_TEXTURE);
//        glActiveTexture(GL_TEXTURE3);
//        glPushMatrix();
//        glMultMatrixf(&m_matrix[0][0]);

        //printf("pos: %f %f %f \n",m_matrix.m_posit.m_x, m_matrix.m_posit.m_y, m_matrix.m_posit.m_z);


        for(unsigned int i=0; i<mesh.size(); i++)
        {
            if(material.at(i) != NULL)
            {
                glActiveTexture( GL_TEXTURE0 );
                glBindTexture(GL_TEXTURE_2D, material.at(i)->diffuseMap);

                glActiveTexture( GL_TEXTURE1 );
                glBindTexture(GL_TEXTURE_2D, material.at(i)->normalMap);

                glActiveTexture( GL_TEXTURE2 );
                glBindTexture(GL_TEXTURE_2D, material.at(i)->heightMap);

                glMaterialfv(GL_FRONT, GL_DIFFUSE, material.at(i)->diffuse);
                glMaterialfv(GL_FRONT, GL_AMBIENT, material.at(i)->ambient);
                glMaterialfv(GL_FRONT, GL_SPECULAR, material.at(i)->specular);
                glMaterialfv(GL_FRONT, GL_SHININESS, material.at(i)->shininess);

                setUniformVariables(material.at(i)->shader,0,1,2,mesh.at(i)->tangent);

                glActiveTexture( GL_TEXTURE3 );
                glBindTexture(GL_TEXTURE_2D, nearestLight->depthTextureId);

                setUniformVariable(material.at(i)->shader,3,"ShadowMap");

                glUseProgram( material.at(i)->shader );

            }
            if(mesh.at(i) != NULL)
            {
                mesh.at(i)->draw();
            }
        }

        glUseProgram( 0 );

        //glPopMatrix();

        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void StaticEntity::CalculateBounds()
{
	dVector minBox = dVector ( 1.0e10f,  1.0e10f,  1.0e10f, 1.0f);
	dVector maxBox = dVector (-1.0e10f, -1.0e10f, -1.0e10f, 1.0f);

	dFloat val;
    unsigned int i, u;

    for(i=0; i<mesh.size(); i++)
    {
        for(u=0; u<mesh.at(i)->numVertices; u++)
        {

            val = mesh[i]->vertex[u].x;
            printf("val: %.1f \n",val);
            minBox.m_x = (val < minBox.m_x) ? val : minBox.m_x;
            maxBox.m_x = (val > maxBox.m_x) ? val : maxBox.m_x;

            val = mesh[i]->vertex[u].y;
            minBox.m_y = (val < minBox.m_y) ? val : minBox.m_y;
            maxBox.m_y = (val > maxBox.m_y) ? val : maxBox.m_y;

            val = mesh[i]->vertex[u].z;
            minBox.m_z = (val < minBox.m_z) ? val : minBox.m_z;
            maxBox.m_z = (val > maxBox.m_z) ? val : maxBox.m_z;
        }
    }


    this->m_minBox = minBox;
    this->m_maxBox = maxBox;
}
