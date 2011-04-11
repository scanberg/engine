#ifndef Vector3f_H
#define Vector3f_H

#include <math.h>

using namespace std;

class Vector3f
{
public:
    float x,y,z;

    Vector3f();
    Vector3f(float _x, float _y, float _z);

    Vector3f& operator += (const Vector3f& v);

    float length();
    float lengthSquared();

private:
    //float data[3];
};

typedef Vector3f *pVector3f;

Vector3f operator + (const Vector3f &a, const Vector3f &b);
Vector3f operator - (const Vector3f &a, const Vector3f &b);
Vector3f operator * (const Vector3f &a, const Vector3f &b); //Component wise

Vector3f operator * (float f, const Vector3f& v);
Vector3f operator * (const Vector3f& v, float f);
Vector3f operator / (const Vector3f& v, float f);

float dot(const Vector3f &v1, const Vector3f &v2);
Vector3f cross(const Vector3f &v1, const Vector3f &v2);
Vector3f normalize(const Vector3f &v);



#endif
