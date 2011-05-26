#ifndef Camera_H
#define Camera_H
#include "Math.h"

class Camera
{
public:
    static Camera *getActiveCamera() { return activeCamera; }
    static void setActiveCamera(Camera &cam) { Camera::activeCamera = &cam; }
    static void setActiveCamera(Camera *cam) { Camera::activeCamera = cam; }
    static void setUp();
    static void setUpInverse();
    static glm::mat4 getViewMatrix();
    static glm::mat4 getInvViewMatrix();

    Camera();
    void setPosition(float _x, float _y, float _z);
    void setDirection(float _rx, float _ry, float _rz);

    glm::vec3 pos;
    glm::vec3 dir;
private:
    static Camera* activeCamera;
};

#endif
