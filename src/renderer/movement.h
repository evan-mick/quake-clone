#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "camera.h"

class Movement
{
public:
    Movement(Camera& cam);
    void forward(float dist);
    void backward(float dist);
    void left(float dist);
    void right(float dist);
    void up(float dist);
    void down(float dist);
private:
    Camera& cam_;

};

#endif // MOVEMENT_H
