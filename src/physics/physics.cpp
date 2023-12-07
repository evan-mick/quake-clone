#include "physics.h"

Physics::Physics(float tickTime)
{
    phys = this;
    m_timer.setAndResetTimer(tickTime);
    this->m_tickTime = tickTime;
}


void Physics::tryRunStep(struct ECS* e, entity_t ent, float delta_seconds) {
    phys->m_timer.increment(delta_seconds);

    // These are both required by run step, no null check needed, if null something is wrong
    PhysicsData* physDat = static_cast<PhysicsData*>(e->getComponentData(ent, FLN_PHYSICS));
    Transform* transform = static_cast<Transform*>(e->getComponentData(ent, FLN_TRANSFORM));
    assert(physDat != nullptr && transform != nullptr);


    if (phys->m_timer.finishedThenResetTime()) {
        // Run the simulation
        // IF current entity has moved since last tick (last_pos != current_pos)
        // Iterate through all static objects
        //      IF collides
        //          IF Not trigger
        //              Offset both objects based on type of collision
        //          IF type to world collision registered
        //              run it
        //
        // Iterate through all other ECS objects
        // If they also have physics and have a collision, run a collision check.
        //      IF not in combined entity ids AND collides
        //          IF Not trigger
        //              Offset both objects based on type of collision
        //          IF type registered
        //              run collision logic on each entity for the registered type,
        //              pass in both entity id
        //          Add collision to set of registered collisions, combined bitmask of ordered entity ids

        // OPTIMIZATIONS: static stuff could be accelerated with KD-tree or similar
        // entities would need more dynamic system
    }
}

void Physics::registerType(ent_type_t type, collision_response_t response) {
    // no bounds checking because array size correlates with size of ent_type
    m_typeToResponse[type] = response;
}
