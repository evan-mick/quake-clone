#ifndef PHYSICS_H
#define PHYSICS_H

#include "core/ecs.h"
#include "core/timer.h"
#include "game_types.h"
#include "scene/scenedata.h"
#include <unordered_set>

// https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
// basically what we need to do

// Collision types
const uint8_t COL_NONE = 0;
const uint8_t COL_AABB = 1;
const uint8_t COL_SPHERE = 2;

// Pass in ecs and entity ids, then will return the translation offset (or will be zero if not colliding)
typedef glm::vec3 (*collision_response_t)(struct ECS*, entity_t my_ent, entity_t other_ent, bool world);


//struct AABB {
//    glm::vec3 min;
//    glm::vec3 max;
//};

//struct SPHERE {
//    glm::vec3 min;
//    glm::vec3 max;
//};


class Physics
{
public:
    // Creates Physics object, of note, this is a singleton
    Physics(float tickTime);


    inline void startFrame() {
        m_frameRun = false;
        if (m_timer.finished())
            m_timer.reset();
    }

    // Attempts to run a step with the physics singleton
    void tryRunStep(struct ECS*, entity_t entity_id, float delta_seconds);

    // Registers a type -> response correlation
    // When entities with [type] collide with something, they will run [response]
    // Of note, type is a game level struct, and if the entity doesn't have a type
    // registered, they won't run any special code on collision
    void registerType(ent_type_t type, collision_response_t response);

    // Gets required flags from the physics ob
    inline flags_t getRequiredFlags() {
        return requiredFlags;
    }

    inline void setStaticObs(SceneData* data) {
        m_sceneData = data;
    }

    // Prepares internal data for the next tick
    void Reset();



private:



//    std::vector<RenderObject>* m_staticObs = nullptr;
    SceneData* m_sceneData = nullptr;


    // Arbitrary constructor, gets changed right away in physics constructor
    Timer m_timer = Timer(1/20.f);
    float m_tickTime;
    flags_t requiredFlags = (1 << FLN_PHYSICS) | (1 << FLN_TRANSFORM);

    bool m_frameRun = false;

    // Arbitrary limit on number of types, once again, here so we can use arrays
    std::array<collision_response_t, MAX_TYPES> m_typeToResponse;

    std::array<Transform, MAX_ENTITY> m_previousTransforms;

    // All of this is for storing what collisions have already occured
    std::unordered_set<size_t> m_collisionOccured;

    inline bool checkOccured(entity_t ent, entity_t other) {
        size_t first_check = (ent << sizeof(ent)) | other;
        size_t second_check = (other << sizeof(other)) | ent;
        return m_collisionOccured.count(first_check) || m_collisionOccured.count(second_check);
    }
    inline void addOccured(entity_t ent, entity_t other) {
        size_t first_check = (ent << sizeof(ent)) | other;
        m_collisionOccured.insert(first_check);
    }

    bool AABBtoAABBIntersect(Transform* transform, PhysicsData* physics, Transform* otherTransform, PhysicsData* otherPhysics, bool slide);


//    void runStep(struct ECS*, entity_t entity_id, float delta_seconds);
};

#endif // PHYSICS_H
