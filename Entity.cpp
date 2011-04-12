#define GLEW_STATIC
#include <GL/glew.h>

#include <GL/glfw.h>
#include <math.h>
#include <iostream>
#include "Entity.h"
#include "Mesh.h"
#include "Material.h"

using std::cout;

/* CREDS TO STEGU */
void drawTexturedSphere(float r, int segs) {
  int i, j;
  float x, y, z, z1, z2, R, R1, R2;

  // Top cap
  glBegin(GL_TRIANGLE_FAN);
  glNormal3f(0,0,1);
  glTexCoord2f(0.5f,1.0f); // This is an ugly (u,v)-mapping singularity
  glVertex3f(0,0,r);
  z = cos(M_PI/segs);
  R = sin(M_PI/segs);
    for(i = 0; i <= 2*segs; i++) {
      x = R*cos(i*2.0*M_PI/(2*segs));
      y = R*sin(i*2.0*M_PI/(2*segs));
      glNormal3f(x, y, z);
      glTexCoord2f((float)i/(2*segs), 1.0f-1.0f/segs);
      glVertex3f(r*x, r*y, r*z);
    }
  glEnd();

  // Height segments
  for(j = 1; j < segs-1; j++) {
    z1 = cos(j*M_PI/segs);
    R1 = sin(j*M_PI/segs);
    z2 = cos((j+1)*M_PI/segs);
    R2 = sin((j+1)*M_PI/segs);
    glBegin(GL_TRIANGLE_STRIP);
    for(i = 0; i <= 2*segs; i++) {
      x = R1*cos(i*2.0*M_PI/(2*segs));
      y = R1*sin(i*2.0*M_PI/(2*segs));
      glNormal3f(x, y, z1);
      glTexCoord2f((float)i/(2*segs), 1.0f-(float)j/segs);
      glVertex3f(r*x, r*y, r*z1);
      x = R2*cos(i*2.0*M_PI/(2*segs));
      y = R2*sin(i*2.0*M_PI/(2*segs));
      glNormal3f(x, y, z2);
      glTexCoord2f((float)i/(2*segs), 1.0f-(float)(j+1)/segs);
      glVertex3f(r*x, r*y, r*z2);
    }
    glEnd();
  }

  // Bottom cap
  glBegin(GL_TRIANGLE_FAN);
  glNormal3f(0,0,-1);
  glTexCoord2f(0.5f, 1.0f); // This is an ugly (u,v)-mapping singularity
  glVertex3f(0,0,-r);
  z = -cos(M_PI/segs);
  R = sin(M_PI/segs);
    for(i = 2*segs; i >= 0; i--) {
      x = R*cos(i*2.0*M_PI/(2*segs));
      y = R*sin(i*2.0*M_PI/(2*segs));
      glNormal3f(x, y, z);
      glTexCoord2f(1.0f-(float)i/(2*segs), 1.0f/segs);
      glVertex3f(r*x, r*y, r*z);
    }
  glEnd();
}

Entity::Entity()
{
    parent = NULL;
    parentIndex = -1;
    position.x=position.y=position.z=0.0;
    rotation=position;
    visible=true;
    scale=1.0;
}

Entity::~Entity() {};
void Entity::Remove()
{
    unsigned short i=NumChildren();
    while(i-->0)
    {
        child.at(i)->Remove();
    }
    delete this;
};
void Entity::AddChild(Entity &obj)
{
    child.push_back(&obj);
    obj.parent=this;
}
void Entity::AddChild(Entity *obj)
{
    child.push_back(obj);
    obj->parent=this;
}
void Entity::SetPosition(float px, float py, float pz)
{
    position.x=px;
    position.y=py;
    position.z=pz;
}
void Entity::SetRotation(float rx, float ry, float rz)
{
    rotation.x=rx;
    rotation.y=ry;
    rotation.z=rz;
}
void Entity::SetScale(float s) { scale = s; }
void Entity::SetVisibility(bool b)
{
    visible=b;
}
void Entity::SetName(const string& s)
{
    name = s;
}

int Entity::NumChildren()
{
    return child.size();
}

void Entity::Draw()
{
    if(visible)
    {
        glPushMatrix();

        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotation.x, 1.0f, 0.0f, 0.0f);
        glRotatef(rotation.y, 0.0f, 1.0f, 0.0f);
        glRotatef(rotation.z, 0.0f, 0.0f, 1.0f);
        glScalef(scale,scale,scale);

        cit=child.begin();
        while(cit!=child.end())
        {
            (*cit)->Draw();
            cit++;
        }

        glPopMatrix();
    }
}

MeshEntity::MeshEntity()
{
    mesh=NULL;
    material=NULL;
}

MeshEntity::~MeshEntity()
{
    delete mesh;
    delete material;
}

void SphereEntity::Draw()
{
    if(visible)
    {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotation.x, 1.0f, 0.0f, 0.0f);
        glRotatef(rotation.y, 0.0f, 1.0f, 0.0f);
        glRotatef(rotation.z, 0.0f, 0.0f, 1.0f);

        cit=child.begin();
        while(cit!=child.end())
        {
            (*cit)->Draw();
            cit++;
        }

        drawTexturedSphere(radius,10);

        glPopMatrix();
    }
}

void SphereEntity::SetRadius(float r)
{
    radius = r;
}

void StaticEntity::Draw()
{
    if(visible)
    {
        glPushMatrix();

        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotation.x, 1.0f, 0.0f, 0.0f);
        glRotatef(rotation.y, 0.0f, 1.0f, 0.0f);
        glRotatef(rotation.z, 0.0f, 0.0f, 1.0f);
        glScalef(scale,scale,scale);

        cit=child.begin();
        while(cit!=child.end())
        {
            (*cit)->Draw();
            cit++;
        }

        //The actual draw

        if(material != NULL)
        {
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture(GL_TEXTURE_2D, material->diffuseMap);

            glActiveTexture( GL_TEXTURE1 );
            glBindTexture(GL_TEXTURE_2D, material->normalMap);

            glActiveTexture( GL_TEXTURE2 );
            glBindTexture(GL_TEXTURE_2D, material->heightMap);

            glMaterialfv(GL_FRONT, GL_DIFFUSE, material->diffuse);
            glMaterialfv(GL_FRONT, GL_AMBIENT, material->ambient);
            glMaterialfv(GL_FRONT, GL_SPECULAR, material->specular);
            glMaterialfv(GL_FRONT, GL_SHININESS, material->shininess);

            setUniformVariables(material->shader,0,1,2,mesh->tangent);

            glUseProgram( material->shader );
        }
        if(mesh != NULL)
        {
            mesh->draw();
        }

        glUseProgram( 0 );

        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
