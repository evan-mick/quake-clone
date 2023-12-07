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
    }
}


