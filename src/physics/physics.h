#ifndef PHYSICS_H
#define PHYSICS_H

#include "core/ecs.h"
#include "core/timer.h"
#include "game_types.h"

class Physics
{
public:
    // Creates Physics object, of note, this is a singleton
    Physics(float tickTime);

    // Attempts to run a step with the physics singleton
    static void tryRunStep(struct ECS*, entity_t entity_id, float delta_seconds);

    // Gets required flags from the physics ob
    inline flags_t getRequiredFlags() {
        return requiredFlags;
    }

private:
    // The singleton
    static Physics* phys;

    Timer m_timer = Timer(1/20.f);
    float m_tickTime;
    flags_t requiredFlags = (1 << FLN_PHYSICS) | (1 << FLN_TRANSFORM);
//    void runStep(struct ECS*, entity_t entity_id, float delta_seconds);
};

#endif // PHYSICS_H
