#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glfw.h>

#include "Mesh.h"

#define SETELEMENT(arr, stride, i, u, v, nx, ny, nz, x, y, z) \
    (arr)[(i)*(stride)] = u; \
    (arr)[(i)*(stride)+1] = v; \
    (arr)[(i)*(stride)+2] = nx; \
    (arr)[(i)*(stride)+3] = ny; \
    (arr)[(i)*(stride)+4] = nz; \
    (arr)[(i)*(stride)+5] = x; \
    (arr)[(i)*(stride)+6] = y; \
    (arr)[(i)*(stride)+7] = z

using namespace std;

//Hjälpfunktioner
float toFloat(string s);
int toInt(string s);
void normalize(glm::vec3 &v);
void normalizeXYZ(float &x, float &y, float &z);
void fillBuffers(float **varray, int **iarray, Mesh* mesh);
void calculateTangent(glm::vec3 &tangent, glm::vec3 v1, glm::vec3 v2, glm::vec2 st1, glm::vec2 st2);

void Mesh::init(unsigned int verts, unsigned int faces)
{
    numVertices=verts;
    numFaces=faces;
    vertex = new Vertex[numVertices];
    tangent = new glm::vec3[numVertices];
    face = new Face[numFaces];
}

void Mesh::scale(float s)
{
    for(unsigned int i=0; i<numVertices; i++)
    {
        vertex[i].x *= s;
        vertex[i].y *= s;
        vertex[i].z *= s;
    }
}

glm::vec3 createVec3(const Vertex &a,const Vertex &b)
{
    glm::vec3 v;
    v.x=b.x-a.x;
    v.y=b.y-a.y;
    v.z=b.z-a.z;
    return v;
}

void Mesh::calculateNormals()
{
    unsigned int sharedFaces[this->numVertices];
    unsigned int i;
    float faces;
    glm::vec3 normal,tang,a,b;

    center = glm::vec3(0.0f);

    for (i=0; i<this->numVertices; i++)
    {
        vertex[i].nx=vertex[i].ny=vertex[i].nz=0.0f;
        tangent[i].x=tangent[i].y=tangent[i].z=0.0f;
        sharedFaces[i]=0;
        center.x += vertex[i].x;
        center.y += vertex[i].y;
        center.z += vertex[i].z;
    }

    center /= (float)numVertices;

    for (i=0; i<this->numFaces; i++)
    {
        a=createVec3(vertex[face[i].point[0]],vertex[face[i].point[2]]);
        b=createVec3(vertex[face[i].point[0]],vertex[face[i].point[1]]);

        normal = glm::cross(b,a);

        glm::vec2 st1, st2;

        st1.x = vertex[face[i].point[2]].u - vertex[face[i].point[0]].u;
        st1.y = vertex[face[i].point[2]].v - vertex[face[i].point[0]].v;

        st2.x = vertex[face[i].point[1]].u - vertex[face[i].point[0]].u;
        st2.y = vertex[face[i].point[1]].v - vertex[face[i].point[0]].v;

        calculateTangent(tang, a, b, st1, st2);

        normal = glm::normalize(normal);
        tang = glm::normalize(tang);

        vertex[face[i].point[0]].nx += normal.x;
        vertex[face[i].point[0]].ny += normal.y;
        vertex[face[i].point[0]].nz += normal.z;

        vertex[face[i].point[1]].nx += normal.x;
        vertex[face[i].point[1]].ny += normal.y;
        vertex[face[i].point[1]].nz += normal.z;

        vertex[face[i].point[2]].nx += normal.x;
        vertex[face[i].point[2]].ny += normal.y;
        vertex[face[i].point[2]].nz += normal.z;

        tangent[face[i].point[0]].x += tang.x;
        tangent[face[i].point[0]].y += tang.y;
        tangent[face[i].point[0]].z += tang.z;

        tangent[face[i].point[1]].x += tang.x;
        tangent[face[i].point[1]].y += tang.y;
        tangent[face[i].point[1]].z += tang.z;

        tangent[face[i].point[2]].x += tang.x;
        tangent[face[i].point[2]].y += tang.y;
        tangent[face[i].point[2]].z += tang.z;

        sharedFaces[face[i].point[0]]++;
        sharedFaces[face[i].point[1]]++;
        sharedFaces[face[i].point[2]]++;
    }

    for (i=0; i<this->numVertices; i++)
    {
        if(sharedFaces[i]>0)
        {
            faces = (float)sharedFaces[i];

            vertex[i].nx /= faces;
            vertex[i].ny /= faces;
            vertex[i].nz /= faces;

            tangent[i].x /= faces;
            tangent[i].y /= faces;
            tangent[i].z /= faces;
        }

        normalizeXYZ(vertex[i].nx,vertex[i].ny,vertex[i].nz);
        tangent[i]=glm::normalize(tangent[i]);
    }

}

// *CREDITS TO STEGU*
void Mesh::createBuffers()
{
    float *varray;
    int   *iarray;
    fillBuffers(&varray,&iarray,this);

    glGenBuffers(1, &vBufferID);
    glGenBuffers(1, &iBufferID);

    glBindBuffer(GL_ARRAY_BUFFER, vBufferID);
    glBufferData(GL_ARRAY_BUFFER, this->numVertices*8*sizeof(float), varray, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->numFaces*3*sizeof(int), iarray, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::draw()
{
    glBindBuffer(GL_ARRAY_BUFFER, this->vBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->iBufferID);

    glInterleavedArrays(GL_T2F_N3F_V3F, 0, (GLubyte*)NULL);
    glDrawElements(GL_TRIANGLES, 3*this->numFaces, GL_UNSIGNED_INT, (GLubyte*)NULL);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void calculateTangent(glm::vec3 &tangent, glm::vec3 v1, glm::vec3 v2, glm::vec2 st1, glm::vec2 st2)
{
    float coef = 1.0/ (st1.x * st2.y - st2.x * st1.y);

    tangent.x = coef * ((v1.x * st2.y)  + (v2.x * -st1.y));
    tangent.y = coef * ((v1.y * st2.y)  + (v2.y * -st1.y));
    tangent.z = coef * ((v1.z * st2.y)  + (v2.z * -st1.y));
}

// *CREDITS TO STEGU*
void fillBuffers(float **varray, int **iarray, Mesh* mesh)
{
    unsigned int i=0;
    int stride = 8;
    *varray = (float*)malloc(mesh->numVertices*sizeof(Vertex));
    *iarray = (int*)malloc(mesh->numFaces*sizeof(Face));

    //Förmodligen använda struct direkt istället för setelment för att stega igenom minnet
    for(i=0; i<mesh->numVertices; i++)
    {
        SETELEMENT(*varray, stride, i,
                   mesh->vertex[i].u, mesh->vertex[i].v,
                   mesh->vertex[i].nx, mesh->vertex[i].ny, mesh->vertex[i].nz,
                   mesh->vertex[i].x, mesh->vertex[i].y, mesh->vertex[i].z);
    }

    for(i=0; i<mesh->numFaces; i++)
    {
        (*iarray)[3*i]   = mesh->face[i].point[0];
        (*iarray)[3*i+1] = mesh->face[i].point[1];
        (*iarray)[3*i+2] = mesh->face[i].point[2];
    }

}

float toFloat(string s)
{
    return atof(s.c_str());
}

int toInt(string s)
{
    return atoi(s.c_str());
}
