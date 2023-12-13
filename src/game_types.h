#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "glm/glm.hpp"
#include <string>
#include "core/ecs.h"



// COMPONENT FLAG NUMBERS
// represents where in the bitmask they are
constexpr uint32_t FLN_TYPE = 10;
constexpr uint32_t FLN_INPUT = 1;
constexpr uint32_t FLN_TRANSFORM = 2;
constexpr uint32_t FLN_RENDER = 3;
constexpr uint32_t FLN_PHYSICS = 4;
constexpr uint32_t FLN_COLLISION = 5;
constexpr uint32_t FLN_PROJECTILE = 6;

constexpr uint32_t FLN_TESTKILL = 29;
constexpr uint32_t FLN_TEST = 30;


const uint32_t DSCREEN_WIDTH = 1200;
const uint32_t DSCREEN_HEIGHT = 900;
constexpr float RAW_FOV = 90;
constexpr float FOV = glm::radians(RAW_FOV); //* (float)DSCREEN_HEIGHT/(float)DSCREEN_WIDTH;
const float NEAR_PLANE = 0.45f;
const float FAR_PLANE = 200.0f;

// INPUT FLAGS
const int IN_FORWARD = 0;
const int IN_BACK = 1;
const int IN_LEFT = 2;
const int IN_RIGHT = 3;
const int IN_SHOOT = 4;
const int IN_JUMP = 5;

// GAME LOGIC CONSTANTS
const uint8_t TICKS_PER_SECOND = 90; // 60 normally
constexpr float TICK_RATE = 1.f/TICKS_PER_SECOND;


typedef unsigned char ent_type_t;
// COMPONENT STRUCTS
struct TypeData {
    ent_type_t type;
};

inline ent_type_t getType(ECS* e, entity_t ent) {
    void* type = e->getComponentData(ent, FLN_TYPE);
    if (type == nullptr)
        return 0;
    return static_cast<TypeData*>(type)->type;
}
inline void trySetType(ECS* e, entity_t ent, ent_type_t t) {
    void* type = e->getComponentData(ent, FLN_TYPE);
    if (type == nullptr)
        return;
    static_cast<TypeData*>(type)->type = t;
}



constexpr ent_type_t ET_NONE = 0;
constexpr ent_type_t ET_PLAYER = 1;
constexpr ent_type_t ET_PROJ = 2;

constexpr uint8_t MAX_TYPE_VAL = -1;
constexpr uint16_t MAX_TYPES = MAX_TYPE_VAL + 1;

const glm::vec3 GRAVITY { 0, -9.8f, 0 };


typedef uint8_t input_t;
struct InputData {
    input_t dat;
    input_t last_dat;
    float x_look = 0.f;
    float y_look = 0.f;
};

struct Transform {
    glm::vec3 pos;
    glm::vec3 scale;
    glm::vec3 rot;
};
inline Transform* getTransform(ECS* e, entity_t ent) {
    return static_cast<Transform*>(e->getComponentData(ent, FLN_TRANSFORM));
}

struct PhysicsData {
    glm::vec3 vel;
    glm::vec3 accel;
    bool grounded = false;
};
inline PhysicsData* getPhys(ECS* e, entity_t ent) {
    return static_cast<PhysicsData*>(e->getComponentData(ent, FLN_PHYSICS));
}

struct CollisionData {
    int8_t col_type; // Positive => physical, Negative => trigger, abs(col_type) => collision type
};
inline CollisionData* getCollisionData(ECS* e, entity_t ent) {
    return static_cast<CollisionData*>(e->getComponentData(ent, FLN_COLLISION));
}


struct Projectile {
    float speed;
};

struct Renderable {
    uint8_t model_id;
};

template <typename T>
inline T* getComponentData(ECS* e, entity_t ent, uint32_t flag) {
    return static_cast<T*>(e->getComponentData(ent, flag));
}

//const int test = sizeof(PhysicsData) + sizeof(Transform) + sizeof(InputData) + sizeof(Renderable);
//const int test2 = sizeof(Transform) + sizeof(Projectile) + sizeof(Renderable);

struct Test {
    float timer = 0.0f;
    int ticks = 0;
};



#endif // GAME_TYPES_H
