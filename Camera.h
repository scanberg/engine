#ifndef Camera_H
#define Camera_H

class fpsCamera
{
public:
    fpsCamera();
    void position(float _x, float _y, float _z);
    void move(float _x, float _y, float _z);
    void rotate(float _rx, float _ry, float _rz);
    void update();
    void setUp();
    void setFollowMouse(bool b);
    bool getFollowMouse(){return followMouse;};
    float x, y, z;
private:
    float rx, ry, rz;
    int mousex, mousey, lastmousex, lastmousey;
    bool followMouse;
};

#endif
