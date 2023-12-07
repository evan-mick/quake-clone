#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "glm/glm.hpp"
#include <string>



// COMPONENT FLAG NUMBERS
// represents where in the bitmask they are
constexpr uint32_t FLN_TYPE = 0;
constexpr uint32_t FLN_INPUT = 1;
constexpr uint32_t FLN_TRANSFORM = 2;
constexpr uint32_t FLN_RENDER = 3;
constexpr uint32_t FLN_PHYSICS = 4;

constexpr uint32_t FLN_TESTKILL = 29;
constexpr uint32_t FLN_TEST = 30;


// COMPONENT BIT FLAGS
//constexpr u_int32_t FL_INPUT = 1;
//constexpr u_int32_t FL_TRANSFORM = 1 << (FLN_TRANSFORM);
//constexpr u_int32_t FL_RENDER = 1 << (FLN_RENDER);
//constexpr u_int32_t FL_PHYSICS = 1 << (FLN_PHYSICS);

//constexpr u_int32_t FL_TESTKILL = 1 << (FLN_TESTKILL);
//constexpr u_int32_t FL_TEST = 1 << (FLN_TEST);

// GAME LOGIC CONSTANTS
const uint8_t TICKS_PER_SECOND = 20;
constexpr float TICK_RATE = 1.f/TICKS_PER_SECOND;

// COMPONENT STRUCTS
struct TypeData {
    uint8_t type;
};


struct InputData {
    uint8_t dat;
    // float x_rot
    // float y_rot
};

struct Transform {
    glm::vec3 pos;
    // float x_rot
    // float y_rot
};

struct PhysicsData {
    glm::vec3 vel;
    glm::vec3 accel;
    // float x_rot
    // float y_rot
};

struct Renderable {
    uint8_t model_id;

};

struct Test {
    std::string to_print;
    float timer = 0.0f;
    int ticks = 0;
};


#endif // GAME_TYPES_H
