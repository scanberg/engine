#define GLEW_STATIC

#include <GL/glew.h>

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <math.h>
#include <GL/glfw.h>
#include "Mesh.h"
//#include "Extensions.h"

using namespace std;

//Hjälpfunktioner
float toFloat(string s);
int toInt(string s);
void Normalize(Vector3f &v);
void fillBuffers(float **varray, int **iarray, Mesh* mesh);
void CalculateTangent(Vector3f &tangent,Vector3f v1, Vector3f v2, Vector2f st1, Vector2f st2);

void Mesh::Init(unsigned int verts, unsigned int faces)
{
    numVertices=verts;
    numFaces=faces;
    vertex = new Vertex[numVertices];
    tangent = new Vector3f[numVertices];
    face = new Face[numFaces];
}

void Normalize(Vector3f &v)
{
    float length = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    if(length>0.0f)
    {
        length = 1.0/length;
        v.x *= length;
        v.y *= length;
        v.z *= length;
    }
}

void Normalize(float &x, float &y, float &z)
{
    float length = sqrt(x*x + y*y + z*z);
    if(length>0.0f)
    {
        length = 1.0/length;
        x *= length;
        y *= length;
        z *= length;
    }
}

Vector3f CreateVector(Vertex a, Vertex b)
{
    Vector3f v;
    v.x=b.x-a.x;
    v.y=b.y-a.y;
    v.z=b.z-a.z;

    return v;
}

Vector3f VectorProduct(Vector3f a, Vector3f b)
{
    Vector3f n;
    n.x=(a.y * b.z) - (a.z * b.y);
    n.y=(a.z * b.x) - (a.x * b.z);
    n.z=(a.x * b.y) - (a.y * b.x);
    return n;
}

void Mesh::CalculateNormals()
{
    unsigned int sharedFaces[this->numVertices];
    unsigned int i;
    float faces;
    Vector3f normal,tang, a, b;

    for (i=0; i<this->numVertices; i++)
    {
        vertex[i].nx=vertex[i].ny=vertex[i].nz=0.0;
        tangent[i].x=tangent[i].y=tangent[i].z=0.0;
        sharedFaces[i]=0;
    }

    for (i=0; i<this->numFaces; i++)
    {
        a=CreateVector(vertex[face[i].point[0]],vertex[face[i].point[2]]);
        b=CreateVector(vertex[face[i].point[0]],vertex[face[i].point[1]]);
        normal=VectorProduct(b,a);

        Vector2f st1, st2;
        st1.u = vertex[face[i].point[2]].u - vertex[face[i].point[0]].u;
        st1.v = vertex[face[i].point[2]].v - vertex[face[i].point[0]].v;

        st2.u = vertex[face[i].point[1]].u - vertex[face[i].point[0]].u;
        st2.v = vertex[face[i].point[1]].v - vertex[face[i].point[0]].v;

        CalculateTangent(tang, a, b, st1, st2);
        Normalize(normal);
        Normalize(tang);

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

        Normalize(vertex[i].nx,vertex[i].ny,vertex[i].nz);
        Normalize(tangent[i]);
    }

}

void CalculateTangent(Vector3f &tangent,Vector3f v1, Vector3f v2, Vector2f st1, Vector2f st2)
{
    float coef = 1.0/ (st1.u * st2.v - st2.u * st1.v);

    tangent.x = coef * ((v1.x * st2.v)  + (v2.x * -st1.v));
    tangent.y = coef * ((v1.y * st2.v)  + (v2.y * -st1.v));
    tangent.z = coef * ((v1.z * st2.v)  + (v2.z * -st1.v));
}

// *CREDITS TO STEGU*
void Mesh::CreateBuffers()
{
    float *varray;
    int   *iarray;
    fillBuffers(&varray,&iarray,this);

    glGenBuffers(1, &vBufferID);
    glGenBuffers(1, &iBufferID);

    cout<<"vBuffer: "<<vBufferID<<endl;
    cout<<"iBuffer: "<<iBufferID<<endl;

    glBindBuffer(GL_ARRAY_BUFFER, vBufferID);
    glBufferData(GL_ARRAY_BUFFER, this->numVertices*8*sizeof(float), varray, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->numFaces*3*sizeof(int), iarray, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// *CREDITS TO STEGU*
void fillBuffers(float **varray, int **iarray, Mesh* mesh)
{
    unsigned int i=0;
    int stride = 8;
    *varray = (float*)malloc(mesh->numVertices*sizeof(Vertex));
    *iarray = (int*)malloc(mesh->numFaces*sizeof(Face));

#define SETELEMENT(arr, stride, i, u, v, nx, ny, nz, x, y, z) \
    (arr)[(i)*(stride)] = u; \
    (arr)[(i)*(stride)+1] = v; \
    (arr)[(i)*(stride)+2] = nx; \
    (arr)[(i)*(stride)+3] = ny; \
    (arr)[(i)*(stride)+4] = nz; \
    (arr)[(i)*(stride)+5] = x; \
    (arr)[(i)*(stride)+6] = y; \
    (arr)[(i)*(stride)+7] = z

    //Förmodligen använda struct direkt istället för setelment
    for(i=0; i<mesh->numVertices; i++)
    {
        SETELEMENT(*varray, stride, i,
                   mesh->vertex[i].u, mesh->vertex[i].v,
                   mesh->vertex[i].nx, mesh->vertex[i].ny, mesh->vertex[i].nz,
                   mesh->vertex[i].x, mesh->vertex[i].y, mesh->vertex[i].z);
    }

#undef SETELEMENT

    for(i=0; i<mesh->numFaces; i++)
    {
        (*iarray)[3*i]   = mesh->face[i].point[0];
        (*iarray)[3*i+1] = mesh->face[i].point[1];
        (*iarray)[3*i+2] = mesh->face[i].point[2];
    }

}
void Mesh::Draw()
{
    glBindBuffer(GL_ARRAY_BUFFER, this->vBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->iBufferID);

    glInterleavedArrays(GL_T2F_N3F_V3F, 0, (GLubyte*)NULL);
    glDrawElements(GL_TRIANGLES, 3*this->numFaces, GL_UNSIGNED_INT, (GLubyte*)NULL);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

float toFloat(string s)
{
    return atof(s.c_str());
}

int toInt(string s)
{
    return atoi(s.c_str());
}
