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
	matrix (1.0f)
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
    matrix[3]=glm::vec4(px,py,pz,1.0f);
}
void Entity::SetRotation(float rx, float ry, float rz)
{
    glm::mat4 mat=glm::gtx::euler_angles::yawPitchRoll(rx,ry,rz);
    mat[3] = matrix[3];
    matrix = mat;
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

//===Particle System domain==
ParticleSystemEntity::ParticleSystemEntity() : pos(0.0f,0.0f,0.0f)
{
	ParticleSystem::Init();
}
void ParticleSystemEntity::Update()
{
	ParticleSystem::Update();
}
void ParticleSystemEntity::DrawFirstPass()
{
	ParticleSystem::Render();
}

NewtonEntity::NewtonEntity() :
	curPosition (0.0f, 0.0f, 0.0f, 1.0f),
	prevPosition (0.0f, 0.0f, 0.0f, 1.0f),
	curRotation (1.0f, 0.0f, 0.0f, 0.0f),
	prevRotation (1.0f, 0.0f, 0.0f, 0.0f)
{
    body=NULL;
}

void NewtonEntity::SetPosition(float px, float py, float pz)
{
    curPosition=prevPosition=matrix[3]=glm::vec4(px,py,pz,1.0f);
}

void NewtonEntity::SetRotation(float rx, float ry, float rz)
{
    glm::mat4 mat = glm::gtx::euler_angles::yawPitchRoll(ry,rz,rx);
    curRotation = glm::gtc::quaternion::quat_cast(mat);
    prevRotation = curRotation;
    matrix = createMat4(curRotation,curPosition);
    if(body)
        NewtonBodySetMatrix( body, &matrix[0][0]);
}

void NewtonEntity::Update(float interpolationParam, NewtonWorld* world)
{
	// Calculate visual Transform by Interpolating between prev and curr State
	glm::vec4 position (prevPosition + (curPosition - prevPosition) * interpolationParam);
	glm::quat rotation = glm::gtc::quaternion::mix(prevRotation,curRotation,interpolationParam);

    matrix = createMat4(rotation,position);

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
        cam->dir.z += (lastmousex - mousex) * 0.2f;
        cam->dir.x += (lastmousey - mousey) * 0.2f;
        if ( cam->dir.x > 180.0f ) cam->dir.x = 180.0f;
        if ( cam->dir.x < 0.0f ) cam->dir.x = 0.0f;

        if ( cam->dir.z > 360.0f ) cam->dir.z -= 360.0f;
        if ( cam->dir.z < -360.0f ) cam->dir.z += 360.0f;
    }
}

void PlayerEntity::UpdatePhysics(NewtonWorld* world, float dt)
{

    Camera* cam=PlayerEntity::camera;
    const float moveSpeed = 100.0*dt;
    const float maxAcceleration = 10.0*dt;
    const float fallAcceleration = 4.0*dt;
    const float jumpStrength = 100.0*dt;

    static bool jumpOk;

    float move = (float)( glfwGetKey('W') - glfwGetKey('S') );
    float strafe = (float)( glfwGetKey('D') - glfwGetKey('A') );

    //Desired velocity representerar den önskvärda hastigheten från input
    glm::vec3 desiredVelocity(strafe,move,0.0f);

    //Normalisera desired om man önskar att gå snett
    if(glm::dot(desiredVelocity,desiredVelocity) > 1.0f)
        desiredVelocity *= 0.70710678f; // 1/sqrt(2)

    //Skala enligt movespeed
    desiredVelocity *= moveSpeed;

    //Skapa en matris för att transformera velocity till samma koordinatsystem som desiredVel är angivet i
    glm::mat4 mat = glm::gtc::matrix_transform::rotate(glm::mat4(1.0f),-cam->dir.z,glm::vec3(0.0f,0.0f,1.0f));

    //Skapa en kopia på velocity och transformera denna.
    glm::vec4 temp(velocity,1.0f);
    temp = mat * temp;

    //Öka på temp enligt desired
    temp.x=Inc(desiredVelocity.x,temp.x,maxAcceleration);
    temp.y=Inc(desiredVelocity.y,temp.y,maxAcceleration);

    //Transformera tillbaks
    mat = glm::gtc::matrix_transform::rotate(glm::mat4(1.0f),cam->dir.z,glm::vec3(0.0f,0.0f,1.0f));
    temp = mat * temp;

    velocity.x=temp.x;
    velocity.y=temp.y;

    if(!glfwGetKey(GLFW_KEY_SPACE))
        jumpOk=true;

    if(!airborne && jumpOk && glfwGetKey(GLFW_KEY_SPACE))
    {
        velocity.z=jumpStrength;
        jumpOk=false;
    }


    velocity.z -= fallAcceleration;
    airborne=true;

    if(velocity.z<=0.0)
    {
        //AlignToGroundConvex();
    }

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

void PlayerEntity::NewtonUpdate (float dt)
{
	// get the entity associated with this rigid body
	PlayerEntity* ent = PlayerEntity::player;

	ent->UpdatePhysics(SceneHandler::world,dt);

    ent->prevPosition = ent->curPosition;
    ent->curPosition = ent->matrix[3];
}

PlayerEntity::PlayerEntity()
{
    PlayerEntity::player = this;
}

MeshObject::MeshObject()
{
    totalVertices=0;
}

MeshObject::~MeshObject()
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

void StaticEntity::DrawGeometry()
{
    if(visible)
    {
        glPushMatrix();

        glMultMatrixf(&matrix[0][0]);
        glScalef(scale,scale,scale);

        for(unsigned int i=0; i<meshObj->mesh.size(); i++)
        {
            if(meshObj->mesh.at(i) != NULL)
            {
                meshObj->mesh.at(i)->draw();
            }
        }
        glPopMatrix();
    }
}

glm::vec3 StaticEntity::ClosestMeshBBoxPoint(unsigned int i)
{
    glm::mat4 matrix = glm::gtc::matrix_inverse::affineInverse(this->matrix);
    glm::vec4 camPos = glm::vec4(Camera::getActiveCamera()->pos,1.0);
    camPos = matrix * camPos;
    glm::vec3 minBox = this->meshObj->mesh.at(i)->minBox * this->scale;
    glm::vec3 maxBox = this->meshObj->mesh.at(i)->maxBox * this->scale;

    camPos.x = glm::core::function::common::clamp(camPos.x, minBox.x, maxBox.x);
    camPos.y = glm::core::function::common::clamp(camPos.y, minBox.y, maxBox.y);
    camPos.z = glm::core::function::common::clamp(camPos.z, minBox.z, maxBox.z);

    return glm::vec3( this->matrix * camPos );
}

void StaticEntity::DrawFirstPass()
{
    GLuint shad;
    glm::vec3 minBBoxPoint;
    glm::vec3 distVec;

    if(visible)
    {
        glPushMatrix();
        glMultMatrixf(&matrix[0][0]);
        glScalef(scale,scale,scale);

        for(unsigned int i=0; i<meshObj->mesh.size(); i++)
        {
            if(meshObj->mesh.at(i))
            {
                if(meshObj->material.at(i))
                {
                    //minBBoxPoint = ClosestMeshBBoxPoint(i);
                    //distVec = minBBoxPoint - Camera::getActiveCamera()->pos;
                    //std::cout<<"dist "<<glm::dot(distVec,distVec)<<std::endl;
                    //shad = SceneHandler::shaderLib.GetShaderFromDistance(meshObj->material.at(i)->type,glm::dot(distVec,distVec));
                    shad=SceneHandler::shaderLib.GetShaderFromType(meshObj->material.at(i)->type);

                    glActiveTexture( GL_TEXTURE0 );
                    glBindTexture(GL_TEXTURE_2D, meshObj->material.at(i)->diffuseMap);
                    setUniform1i(shad,0,"diffuseMap");

                    glActiveTexture( GL_TEXTURE1 );
                    glBindTexture(GL_TEXTURE_2D, meshObj->material.at(i)->normalMap);
                    setUniform1i(shad,1,"normalMap");

                    glActiveTexture( GL_TEXTURE2 );
                    glBindTexture(GL_TEXTURE_2D, meshObj->material.at(i)->specularMap);
                    setUniform1i(shad,2,"specularMap");

                    setAttributeTangent(shad, meshObj->mesh.at(i)->tangent, "tangent");

                    setUniform2f(shad,SceneHandler::near,SceneHandler::far,"cameraRange");

                    Camera *cam = Camera::getActiveCamera();
                    setUniform3f(shad,cam->pos.x,cam->pos.y,cam->pos.z,"cameraPos");
                    setUniform3f(shad,cam->dir.x,cam->dir.y,cam->dir.z,"cameraDir");

                    glMaterialfv(GL_FRONT, GL_DIFFUSE, meshObj->material.at(i)->diffuse);
                    glMaterialfv(GL_FRONT, GL_AMBIENT, meshObj->material.at(i)->ambient);
                    glMaterialfv(GL_FRONT, GL_SPECULAR, meshObj->material.at(i)->specular);
                    glMaterialfv(GL_FRONT, GL_SHININESS, meshObj->material.at(i)->shininess);

                    glUseProgram( shad );
                    meshObj->mesh.at(i)->draw();
                    glUseProgram( 0 );
                }
            }
        }
        glPopMatrix();
    }
}

void StaticEntity::DrawShadow()
{
    if(visible)
    {
        Light *nearestLight=SceneHandler::FindNearestLight(matrix[3].x,
                                                           matrix[3].y,
                                                           matrix[3].z);
        if(nearestLight)
            nearestLight->assignTo(0);

        glPushMatrix();
        glMultMatrixf(&matrix[0][0]);
        glScalef(scale,scale,scale);
        glMatrixMode(GL_TEXTURE);
        glActiveTexture(GL_TEXTURE3);

        glPushMatrix();
        glMultMatrixf(&matrix[0][0]);
        glScalef(scale,scale,scale);

        for(unsigned int i=0; i<meshObj->mesh.size(); i++)
        {
            if(meshObj->material.at(i) != NULL)
            {
                glMaterialfv(GL_FRONT, GL_DIFFUSE, meshObj->material.at(i)->diffuse);
                glMaterialfv(GL_FRONT, GL_AMBIENT, meshObj->material.at(i)->ambient);
                glMaterialfv(GL_FRONT, GL_SPECULAR, meshObj->material.at(i)->specular);
                glMaterialfv(GL_FRONT, GL_SHININESS, meshObj->material.at(i)->shininess);

                glActiveTexture( GL_TEXTURE3 );
                glBindTexture(GL_TEXTURE_2D, nearestLight->depthTextureId);
                setUniform1i(SceneHandler::shadowShader,3,"ShadowMap");
                setUniform1f(SceneHandler::shadowShader, nearestLight->getRadius(),"lightRadius");

                glUseProgram( SceneHandler::shadowShader );

            }
            if(meshObj->mesh.at(i))
            {
                meshObj->mesh.at(i)->draw();
            }
        }

        glUseProgram( 0 );

        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void StaticEntity::Draw()
{
    if(visible)
    {
        Light *nearestLight=SceneHandler::FindNearestLight(matrix[3].x,
                                                           matrix[3].y,
                                                           matrix[3].z);
        if(nearestLight)
            nearestLight->assignTo(0);

        glPushMatrix();
        glMultMatrixf(&matrix[0][0]);
        glScalef(scale,scale,scale);
        glMatrixMode(GL_TEXTURE);
        glActiveTexture(GL_TEXTURE3);

        glPushMatrix();
        glMultMatrixf(&matrix[0][0]);
        glScalef(scale,scale,scale);

        //printf("pos: %f %f %f \n",m_matrix.m_posit.m_x, m_matrix.m_posit.m_y, m_matrix.m_posit.m_z);

        for(unsigned int i=0; i<meshObj->mesh.size(); i++)
        {
            if(meshObj->material.at(i) != NULL)
            {
                glActiveTexture( GL_TEXTURE0 );
                glBindTexture(GL_TEXTURE_2D, meshObj->material.at(i)->diffuseMap);
                setUniform1i(meshObj->material.at(i)->shader,0,"diffuseMap");

                glActiveTexture( GL_TEXTURE1 );
                glBindTexture(GL_TEXTURE_2D, meshObj->material.at(i)->normalMap);
                setUniform1i(meshObj->material.at(i)->shader,1,"normalMap");

                glActiveTexture( GL_TEXTURE2 );
                glBindTexture(GL_TEXTURE_2D, meshObj->material.at(i)->specularMap);
                setUniform1i(meshObj->material.at(i)->shader,2,"specularMap");

                setAttributeTangent(meshObj->material.at(i)->shader, meshObj->mesh.at(i)->tangent, "tangent");

                glMaterialfv(GL_FRONT, GL_DIFFUSE, meshObj->material.at(i)->diffuse);
                glMaterialfv(GL_FRONT, GL_AMBIENT, meshObj->material.at(i)->ambient);
                glMaterialfv(GL_FRONT, GL_SPECULAR, meshObj->material.at(i)->specular);
                glMaterialfv(GL_FRONT, GL_SHININESS, meshObj->material.at(i)->shininess);

                setUniform1f(meshObj->material.at(i)->shader,nearestLight->getRadius(),"lightRadius");

                setUniform2f(meshObj->material.at(i)->shader,(GLfloat)SceneHandler::width,(GLfloat)SceneHandler::height,"bufferSize");

                setUniform1i(meshObj->material.at(i)->shader,SceneHandler::width,"screenWidth");
                setUniform1i(meshObj->material.at(i)->shader,SceneHandler::height,"screenHeight");

                glUseProgram( meshObj->material.at(i)->shader );

            }
            if(meshObj->mesh.at(i))
            {
                meshObj->mesh.at(i)->draw();
            }
        }

        glUseProgram( 0 );

        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void StaticEntity::CalculateBounds()
{
	glm::vec4 l_minBox( 1.0e10f,  1.0e10f,  1.0e10f, 1.0f);
	glm::vec4 l_maxBox(-1.0e10f, -1.0e10f, -1.0e10f, 1.0f);

	float val;
    unsigned int i, u;

    for(i=0; i<meshObj->mesh.size(); i++)
    {
        for(u=0; u<meshObj->mesh.at(i)->numVertices; u++)
        {

            val = meshObj->mesh[i]->vertex[u].x*scale;
            l_minBox.x = (val < l_minBox.x) ? val : l_minBox.x;
            l_maxBox.x = (val > l_maxBox.x) ? val : l_maxBox.x;

            val = meshObj->mesh[i]->vertex[u].y*scale;
            l_minBox.y = (val < l_minBox.y) ? val : l_minBox.y;
            l_maxBox.y = (val > l_maxBox.y) ? val : l_maxBox.y;

            val = meshObj->mesh[i]->vertex[u].z*scale;
            l_minBox.z = (val < l_minBox.z) ? val : l_minBox.z;
            l_maxBox.z = (val > l_maxBox.z) ? val : l_maxBox.z;
        }
    }


    this->minBox = l_minBox;
    this->maxBox = l_maxBox;
}
