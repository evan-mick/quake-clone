#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "glm/glm.hpp"
#include <string>
#include "core/ecs.h"



// COMPONENT FLAG NUMBERS
// represents where in the bitmask they are
constexpr uint32_t FLN_TYPE = 0;
constexpr uint32_t FLN_INPUT = 1;
constexpr uint32_t FLN_TRANSFORM = 2;
constexpr uint32_t FLN_RENDER = 3;
constexpr uint32_t FLN_PHYSICS = 4;
constexpr uint32_t FLN_COLLISION = 5;
constexpr uint32_t FLN_PROJECTILE = 6;

constexpr uint32_t FLN_TESTKILL = 29;
constexpr uint32_t FLN_TEST = 30;


const uint32_t DSCREEN_WIDTH = 800;
const uint32_t DSCREEN_HEIGHT = 600;

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


typedef unsigned char ent_type_t;
// COMPONENT STRUCTS
struct TypeData {
    ent_type_t type;
};
constexpr uint8_t MAX_TYPE_VAL = -1;
constexpr uint16_t MAX_TYPES = MAX_TYPE_VAL + 1;

const glm::vec3 GRAVITY { 0, -9.8f, 0 };


struct InputData {
    uint8_t dat;
};

struct Transform {
    glm::vec3 pos;
    glm::vec3 scale;
    glm::vec3 rot;
};

struct PhysicsData {
    glm::vec3 vel;
    glm::vec3 accel;
};

struct CollisionData {
    int8_t col_type; // Positive => physical, Negative => trigger, abs(col_type) => collision type
};

struct Projectile {
    float speed;
};

struct Renderable {
    uint8_t model_id;
};

//const int test = sizeof(PhysicsData) + sizeof(Transform) + sizeof(InputData) + sizeof(Renderable);
//const int test2 = sizeof(Transform) + sizeof(Projectile) + sizeof(Renderable);

struct Test {
    std::string to_print;
    float timer = 0.0f;
    int ticks = 0;
};


#endif // GAME_TYPES_H
