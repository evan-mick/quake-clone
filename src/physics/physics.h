#ifndef PHYSICS_H
#define PHYSICS_H

#include "core/ecs.h"
#include "core/timer.h"
#include "game_types.h"
#include <unordered_set>

// https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
// basically what we need to do

// Collision types
const uint8_t COL_NONE = 0;
const uint8_t COL_AABB = 1;
const uint8_t COL_SPHERE = 2;

typedef void (*collision_response_t)(struct ECS*, entity_t my_ent, entity_t other_ent);


class Physics
{
public:
    // Creates Physics object, of note, this is a singleton
    Physics(float tickTime);

    // Attempts to run a step with the physics singleton
    static void tryRunStep(struct ECS*, entity_t entity_id, float delta_seconds);

    // Registers a type -> response correlation
    // When entities with [type] collide with something, they will run [response]
    // Of note, type is a game level struct, and if the entity doesn't have a type
    // registered, they won't run any special code on collision
    void registerType(ent_type_t type, collision_response_t response);

    // Gets required flags from the physics ob
    inline flags_t getRequiredFlags() {
        return requiredFlags;
    }

private:
    // The singleton
    static inline Physics* phys;

    // Arbitrary constructor, gets changed right away in physics constructor
    Timer m_timer = Timer(1/20.f);
    float m_tickTime;
    flags_t requiredFlags = (1 << FLN_PHYSICS) | (1 << FLN_TRANSFORM);

    // Arbitrary limit on number of types, once again, here so we can use arrays
    std::array<collision_response_t, MAX_TYPES> m_typeToResponse;

    std::array<Transform, MAX_ENTITY> m_previousTransforms;

    // All of this is for storing what collisions have already occured
    struct Hash {
        entity_t low_ent;
        entity_t high_ent;
    };
    inline Hash createHash(entity_t ent1, entity_t ent2) {
        if (ent1 > ent2)
            return { ent2, ent1 };
        return { ent1, ent2 };
    }
    std::unordered_set<Hash> m_collisionOccured;

//    void runStep(struct ECS*, entity_t entity_id, float delta_seconds);
};

#endif // PHYSICS_H
