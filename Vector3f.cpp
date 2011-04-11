#include "Vector3f.h"

Vector3f::Vector3f()
{
    x=y=z=0.0;
}

Vector3f::Vector3f(float _x, float _y, float _z)
{
    x=_x;
    y=_y;
    z=_z;
}

Vector3f operator+(const Vector3f &a, const Vector3f &b)
{
    Vector3f temp;
    temp.x = a.x + b.x;
    temp.y = a.y + b.y;
    temp.z = a.z + b.z;
    return temp;
}

Vector3f operator-(const Vector3f &a, const Vector3f &b)
{
    Vector3f temp;
    temp.x = a.x - b.x;
    temp.y = a.y - b.y;
    temp.z = a.z - b.z;
    return temp;
}

//Component wise
Vector3f operator * (const Vector3f &a, const Vector3f &b)
{
    Vector3f temp;
    temp.x = a.x * b.x;
    temp.y = a.y * b.y;
    temp.z = a.z * b.z;
    return temp;
}

Vector3f operator * ( float f, const Vector3f& v )
{
    Vector3f temp;
    temp.x = v.x * f;
    temp.y = v.y * f;
    temp.z = v.z * f;
    return temp;
}

Vector3f operator * ( const Vector3f& v, float f )
{
    Vector3f temp;
    temp.x = v.x * f;
    temp.y = v.y * f;
    temp.z = v.z * f;
    return temp;
}

Vector3f operator / ( const Vector3f& v, float f )
{
    Vector3f temp;
    float oneOverf=1.0/f;
    temp.x = v.x * oneOverf;
    temp.y = v.y * oneOverf;
    temp.z = v.z * oneOverf;
    return temp;
}

Vector3f operator += (const Vector3f& v1, const Vector3f& v2)
{
    Vector3f temp;
    temp = v1+v2;
    return temp;
}

Vector3f cross(const Vector3f &v1, const Vector3f &v2)
{
    Vector3f u;
    u.x=(v1.y * v2.z) - (v1.z * v2.y);
    u.y=(v1.z * v2.x) - (v1.x * v2.z);
    u.z=(v1.x * v2.y) - (v1.y * v2.x);
    return u;
}

float dot(const Vector3f &v1, const Vector3f &v2)
{
    return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

Vector3f normalize(const Vector3f &v)
{
    Vector3f u;
    float oneOverLength = 1.0f/sqrt(dot(u,u));
    u = v*oneOverLength;
    return u;
}
