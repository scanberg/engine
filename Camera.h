#ifndef Camera_H
#define Camera_H
#include "Vector3f.h"

#include <Newton.h>
#include <JointLibrary.h>

class Camera
{
public:
    static Camera *getActiveCamera() { return activeCamera; }
    static void setActiveCamera(Camera &cam) { Camera::activeCamera = &cam; }
    static void setActiveCamera(Camera *cam) { Camera::activeCamera = cam; }
    static void setUp();

    Camera();
    void setPosition(float _x, float _y, float _z);
    void setRotation(float _rx, float _ry, float _rz);

    Vector3f pos;
    Vector3f rot;
private:
    static Camera* activeCamera;
};

#endif
