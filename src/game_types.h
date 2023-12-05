#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "glm/glm.hpp"
#include <string>
#include "core/ecs.h"


// COMPONENT FLAG NUMBERS
// represents where in the bitmask they are
constexpr u_int32_t FLN_INPUT = 0;
constexpr u_int32_t FLN_TRANSFORM = 1;
constexpr u_int32_t FLN_RENDER = 2;
constexpr u_int32_t FLN_PHYSICS = 3;

constexpr u_int32_t FLN_TESTKILL = 29;
constexpr u_int32_t FLN_TEST = 30;


// COMPONENT BIT FLAGS
//constexpr u_int32_t FL_INPUT = 1;
//constexpr u_int32_t FL_TRANSFORM = 1 << (FLN_TRANSFORM);
//constexpr u_int32_t FL_RENDER = 1 << (FLN_RENDER);
//constexpr u_int32_t FL_PHYSICS = 1 << (FLN_PHYSICS);

//constexpr u_int32_t FL_TESTKILL = 1 << (FLN_TESTKILL);
//constexpr u_int32_t FL_TEST = 1 << (FLN_TEST);


// COMPONENT STRUCTS
struct InputData {
    u_int8_t dat;
    // float x_rot
    // float y_rot
};

struct Transform {
    glm::vec3 pos;
    // float x_rot
    // float y_rot
};

struct Physics {
    glm::vec3 vel;
    glm::vec3 accel;
    // float x_rot
    // float y_rot
};

struct Renderable {
    u_int8_t model_id;
};

struct Test {
    std::string to_print;
    float timer = 0.0f;
    int ticks = 0;
};


#endif // GAME_TYPES_H
