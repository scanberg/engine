#include "Math.h"

glm::mat4 createMat4(const glm::quat &rotation, const glm::vec4 &position)
{
    glm::mat4 matrix;

	float x2,y2,z2,w2,xy,xz,xw,yz,yw,zw;

	w2 = float (2.0f) * rotation.x * rotation.x;
	x2 = float (2.0f) * rotation.y * rotation.y;
	y2 = float (2.0f) * rotation.z * rotation.z;
	z2 = float (2.0f) * rotation.w * rotation.w;
	assert (abs (w2 + x2 + y2 + z2 - float(2.0f)) < float (1.e-2f));

	xy = float (2.0f) * rotation.y * rotation.z;
	xz = float (2.0f) * rotation.y * rotation.w;
	xw = float (2.0f) * rotation.y * rotation.x;
	yz = float (2.0f) * rotation.z * rotation.w;
	yw = float (2.0f) * rotation.z * rotation.x;
	zw = float (2.0f) * rotation.w * rotation.x;

	matrix[0] = glm::vec4 (float(1.0f) - y2 - z2, xy + zw				 , xz - yw				  , float(0.0f));
	matrix[1]    = glm::vec4 (xy - zw				 , float(1.0f) - x2 - z2, yz + xw				  , float(0.0f));
	matrix[2] = glm::vec4 (xz + yw				 , yz - xw				 , float(1.0f) - x2 - y2 , float(0.0f));

	matrix[3].x = position.x;
	matrix[3].y = position.y;
	matrix[3].z = position.z;
	matrix[3].w = float(1.0f);

	return matrix;
}

void normalizeXYZ(float &x, float &y, float &z)
{
    float length = sqrt(x*x + y*y + z*z);
    if(length>0.0f)
    {
        length = 1.0f/length;
        x *= length;
        y *= length;
        z *= length;
    }
}
