#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

#include <math.h>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <set>
#include <assert.h>
#include "Entity.h"
#include "SceneHandler.h"

Camera* PlayerEntity::camera = NULL;
bool PlayerEntity::followMouse = false;
static glm::vec3 minPush;
static glm::vec3 maxPush;
PlayerEntity* PlayerEntity::player = NULL;

Entity::Entity() :
	matrix (1.0f),
	curPosition (0.0f, 0.0f, 0.0f, 1.0f),
	prevPosition (0.0f, 0.0f, 0.0f, 1.0f),
	curRotation (1.0f, 0.0f, 0.0f, 0.0f),
	prevRotation (1.0f, 0.0f, 0.0f, 0.0f)
{
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
    curPosition=prevPosition=matrix[3]=glm::vec4(px,py,pz,1.0f);
}
void Entity::SetRotation(float rx, float ry, float rz)
{
    curRotation = glm::gtc::quaternion::rotate(curRotation,rx,glm::vec3(1.0,0.0,0.0));
    curRotation = glm::gtc::quaternion::rotate(curRotation,ry,glm::vec3(0.0,1.0,0.0));
    curRotation = glm::gtc::quaternion::rotate(curRotation,rz,glm::vec3(0.0,0.0,1.0));
    prevRotation = curRotation;
}

void Entity::SetVisibility(bool b)
{
    visible=b;
}
void Entity::SetName(const std::string& s)
{
    name = s;
}

void Entity::CalculateBounds()
{

}

void PlayerEntity::Draw()
{
}

float Inc(float target, float current, float stepsize=1.0)
{
    if (current<target)
    {
        current += stepsize;
        current = std::min(current,target);
    }
    else if(current>target)
    {
        current -= stepsize;
        current = std::max(current,target);
    }
    return current;
}

int NewtonBodyCollide(NewtonWorld* world, int maxsize, NewtonBody* body0, NewtonBody* body1, float* contacts, float* normals, float* penetration)
{
      NewtonCollision* collision[2];
      float mat0[16];
      float mat1[16];
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
    float contacts[3*maxSize];
    float normals[3*maxSize];
    float depths[maxSize];
    glm::vec3 pos(matrix[3]);
    glm::vec3 contVec;
    glm::vec3 normal;

    numContacts=NewtonBodyCollide(SceneHandler::world,maxSize,this->body,body,&contacts[0],&normals[0],&depths[0]);

    for(int i=0; i<numContacts; i++)
    {
//        printf("col! pos: (%.1f,%.1f,%.1f) norm: (%.1f,%.1f,%.1f) depth: %.1f \n",
//                contacts[3*i+0],contacts[3*i+1],contacts[3*i+2],
//                normals[3*i+0],normals[3*i+1],normals[3*i+2],depths[i] );

        if(depths[i] > abs(0.0f))
        {

            contVec = glm::vec3(contacts[3*i+0],contacts[3*i+1],contacts[3*i+2]) - pos;
            normal = glm::vec3(normals[3*i+0],normals[3*i+1],normals[3*i+2]);

            //Se till att normalen pekar mot body
            if(glm::dot(contVec,normal)>0.0f)
                normal *= -1.0f;

            //KANSKE BEHÖVS NORMALISERAS?
//            if(glm::dot(normal,normal)>0.0f)
//                normal=glm::normalize(normal);

            if(glm::dot(normal,glm::vec3(0.0,0.0,1.0))>0.5)
                airborne=false;

            normal *= depths[i];

            minPush.x = std::min(minPush.x,normal.x);
            minPush.y = std::min(minPush.y,normal.y);
            minPush.z = std::min(minPush.z,normal.z);
            maxPush.x = std::max(maxPush.x,normal.x);
            maxPush.y = std::max(maxPush.y,normal.y);
            maxPush.z = std::max(maxPush.z,normal.z);
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

void PlayerEntity::Update(float interpolationParam, NewtonWorld* world)
{

    glm::vec4 position(prevPosition + (curPosition - prevPosition)*interpolationParam );

	Camera* cam=PlayerEntity::camera;

    cam->setPosition(position.x,position.y,position.z+eyeHeight);

    static int mousex, mousey, lastmousex, lastmousey;
    bool followMouse=PlayerEntity::followMouse;

    lastmousex = mousex;
    lastmousey = mousey;

    glfwGetMousePos( &mousex, &mousey );

    if(followMouse)
    {
        cam->rot.z += (lastmousex - mousex) * 0.2f;
        cam->rot.x += (lastmousey - mousey) * 0.2f;
        if ( cam->rot.x > 180.0f ) cam->rot.x = 180.0f;
        if ( cam->rot.x < 0.0f ) cam->rot.x = 0.0f;

        if ( cam->rot.z > 360.0f ) cam->rot.z -= 360.0f;
        if ( cam->rot.z < -360.0f ) cam->rot.z += 360.0f;
    }
}

void PlayerEntity::UpdatePhysics(NewtonWorld* world)
{

    Camera* cam=PlayerEntity::camera;
    const float moveSpeed = 1.5;
    const float maxAcceleration = 0.1;

    static bool jumpOk;

    float move = (float)( glfwGetKey('W') - glfwGetKey('S') );
    float strafe = (float)( glfwGetKey('D') - glfwGetKey('A') );

    glm::vec3 desiredVelocity(strafe,move,0.0f);
    glm::vec3 finalVelocity(velocity);

    if(glm::dot(desiredVelocity,desiredVelocity) > 1.0f)
        desiredVelocity *= 0.70710678f;

    glm::mat4 mat = glm::gtc::matrix_transform::rotate(glm::mat4(1.0f),cam->rot.z,glm::vec3(0.0f,0.0f,1.0f));
    glm::vec4 temp(desiredVelocity,1.0f);
    desiredVelocity = glm::vec3(mat * temp);

    desiredVelocity *= moveSpeed;

    finalVelocity.x=Inc(desiredVelocity.x,finalVelocity.x,maxAcceleration);
    finalVelocity.y=Inc(desiredVelocity.y,finalVelocity.y,maxAcceleration);

    velocity.x = finalVelocity.x;
    velocity.y = finalVelocity.y;


    if(!glfwGetKey(GLFW_KEY_SPACE))
        jumpOk=true;

    if(!airborne && jumpOk && glfwGetKey(GLFW_KEY_SPACE))
    {
        velocity.z=1.3;
        jumpOk=false;
    }


    velocity.z -= 0.05;
    airborne=true;

    if(velocity.z<=0.0)
    {
        //AlignToGroundConvex();
    }

    //m_matrix.m_posit += velocity;
    matrix[3].x += velocity.x;
    matrix[3].y += velocity.y;
    matrix[3].z += velocity.z;

    NewtonBodySetMatrix(body,&matrix[0][0]);

    for(int i=0; i<5; i++)
    {
        glm::vec3 g_minBox(matrix[3]+this->minBox);
        glm::vec3 g_maxBox(matrix[3]+this->maxBox);

        minPush = maxPush = glm::vec3(0.0f);

        NewtonWorldForEachBodyInAABBDo(world,&g_minBox[0],&g_maxBox[0],BodyIterator,this);

        matrix[3].x += (minPush.x+maxPush.x);
        matrix[3].y += (minPush.y+maxPush.y);
        matrix[3].z += (minPush.z+maxPush.z);

        NewtonBodySetMatrix(body,&matrix[0][0]);

        glm::vec3 normal;
        normal = minPush + maxPush;

        if(glm::dot(normal,normal)>0.0f)
            normal = glm::normalize(normal);

        velocity = velocity - normal*glm::dot(normal,velocity);
    }

    if(!airborne)
        velocity.z = std::min(velocity.z,0.0f);
}

void PlayerEntity::SetTransform (const NewtonBody* body, const float* matrix, int threadId)
{
	// get the entity associated with this rigid body
	PlayerEntity* ent = PlayerEntity::player;//(PlayerEntity*) NewtonBodyGetUserData(body);

	NewtonWorld* world = SceneHandler::world;//NewtonBodyGetWorld(body);

	ent->UpdatePhysics(world);

    ent->prevPosition = ent->curPosition;
    ent->curPosition = ent->matrix[3];
}

PlayerEntity::PlayerEntity()
{
    PlayerEntity::player = this;
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
    std::set<Material*> mat(material.begin(),material.end());
    std::set<Material*>::const_iterator sit;
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

glm::vec4 Vectorize(const glm::quat &v)
{
    return glm::vec4(v.x,v.y,v.z,v.w);
}

glm::mat3 fixMatrix(const glm::mat3 &m)
{
    glm::vec3 v0(-m[2].z,m[1].z,-m[0].z);
    glm::vec3 v1(m[2].y,-m[1].y,m[0].y);
    glm::vec3 v2(m[2].x,-m[1].x,m[0].x);
    return glm::mat3(v0,v1,v2);
}

void StaticEntity::Update(float interpolationParam, NewtonWorld* world)
{
	// Calculate visual Transform by Interpolating between prev and curr State
	glm::vec4 position (prevPosition + (curPosition - prevPosition) * interpolationParam);
	glm::quat rotation = glm::gtc::quaternion::mix(prevRotation,curRotation,interpolationParam);

    matrix = createMat4(rotation,position);

}

void StaticEntity::DrawGeometry()
{
    if(visible)
    {

        glPushMatrix();

//        glActiveTexture(GL_TEXTURE3);

        glMultMatrixf(&matrix[0][0]);

        for(unsigned int i=0; i<mesh.size(); i++)
        {
            if(mesh.at(i) != NULL)
            {
                mesh.at(i)->draw();
            }
        }

        glPopMatrix();

//        glPopMatrix();
    }
}

void StaticEntity::Draw()
{
    if(visible)
    {
        Light *nearestLight=SceneHandler::FindNearestLight(matrix[3].x,
                                                           matrix[3].y,
                                                           matrix[3].z);
        if(nearestLight!=NULL)
        {
            //cout<<"inte null iaf"<<endl;
            nearestLight->assignTo(0);
        }


        glPushMatrix();

//        glMatrixMode(GL_TEXTURE);
//        glActiveTexture(GL_TEXTURE3);

        glMultMatrixf(&matrix[0][0]);


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

//        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void StaticEntity::CalculateBounds()
{
	glm::vec4 l_minBox( 1.0e10f,  1.0e10f,  1.0e10f, 1.0f);
	glm::vec4 l_maxBox(-1.0e10f, -1.0e10f, -1.0e10f, 1.0f);

	float val;
    unsigned int i, u;

    for(i=0; i<mesh.size(); i++)
    {
        for(u=0; u<mesh.at(i)->numVertices; u++)
        {

            val = mesh[i]->vertex[u].x;
            l_minBox.x = (val < l_minBox.x) ? val : l_minBox.x;
            l_maxBox.x = (val > l_maxBox.x) ? val : l_maxBox.x;

            val = mesh[i]->vertex[u].y;
            l_minBox.y = (val < l_minBox.y) ? val : l_minBox.y;
            l_maxBox.y = (val > l_maxBox.y) ? val : l_maxBox.y;

            val = mesh[i]->vertex[u].z;
            l_minBox.z = (val < l_minBox.z) ? val : l_minBox.z;
            l_maxBox.z = (val > l_maxBox.z) ? val : l_maxBox.z;
        }
    }


    this->minBox = l_minBox;
    this->maxBox = l_maxBox;
}
