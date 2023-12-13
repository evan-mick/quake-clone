#include "physics.h"
#include <chrono>
#include <iostream>

Physics::Physics(float tickTime)
{
    m_timer.setAndResetTimer(tickTime);
    this->m_tickTime = tickTime;
}


void Physics::Reset() {
    m_collisionOccured.clear();
}

void Physics::tryRunStep(struct ECS* e, entity_t my_ent, float delta_seconds) {

    if (!m_frameRun) {
        m_timer.increment(delta_seconds);
        m_frameRun = true;
    }

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




    m_collisionOccured.clear();
    if (m_timer.finished()) {

        auto currentTime = std::chrono::system_clock::now();

        // Convert the time point to a time_t object
        std::time_t currentTimeT = std::chrono::system_clock::to_time_t(currentTime);

//        std::cout << "phys: " << currentTimeT << " " << (int)my_ent << std::endl;
        // These are both required by run step, no null check needed, if null something is wrong
        PhysicsData* physDat = static_cast<PhysicsData*>(e->getComponentData(my_ent, FLN_PHYSICS));
        Transform* transform = static_cast<Transform*>(e->getComponentData(my_ent, FLN_TRANSFORM));
        Renderable* rend = static_cast<Renderable*>(e->getComponentData(my_ent, FLN_RENDER));

        assert(physDat != nullptr && transform != nullptr);

        // I think this ordering is right?
        physDat->vel += physDat->accel;

        auto damp = [](float vel) -> float {
            return abs(vel) > 0.05f ? vel : 0.0f;
        };

        glm::vec3 damped_vel = glm::vec3(damp(physDat->vel.x), damp(physDat->vel.y), damp(physDat->vel.z));
        glm::vec3 damped_accel = glm::vec3(damp(physDat->accel.x), damp(physDat->accel.y), damp(physDat->accel.z));

        // This exists so there is no jitter when the player (or any entity for that matter) is on the ground
        if (physDat->grounded && damped_vel.y <= 0) {
            damped_accel.y = 0;
            damped_vel.y = 0;
        } else if (damped_vel.y > 0) {
            physDat->grounded = false;
        }

        if (glm::length(damped_vel) > 0)
            transform->pos += damped_vel * m_tickTime +
                          damped_accel * m_tickTime * m_tickTime * 0.5f;


        if (transform->pos == (m_previousTransforms[my_ent].pos))
            return;

        m_previousTransforms[my_ent] = *transform;


        if (!e->entityHasComponent(my_ent, FLN_COLLISION))
            return;

        // STATIC OBJECT COLLISIONS
        // OPTIMIZATIONS: static stuff could be accelerated with KD-tree or similar
        // entities would need more dynamic system
        if (m_sceneData != nullptr) {
            physDat->grounded = false;
            for (RenderObject& ob : m_sceneData->shapes) {
                Transform trans {};
                trans.scale.x = glm::length(glm::vec3(ob.ctm[0])); // X-axis scale
                trans.scale.y = glm::length(glm::vec3(ob.ctm[1])); // Y-axis scale
                trans.scale.z = glm::length(glm::vec3(ob.ctm[2])); // Z-axis scale
                trans.pos = glm::vec3(ob.ctm[3]);

                CollisionData* col = getComponentData<CollisionData>(e, my_ent, FLN_COLLISION);
                // (col != nullptr)

                float prev_y = physDat->vel.y;

                if (AABBtoAABBIntersect(getTransform(e, my_ent), physDat, &trans, nullptr, (col->col_type > 0))) {

//                    if (trans.pos.y < getTransform(e, my_ent)->pos.y)
                    if (prev_y != physDat->vel.y)
                        physDat->grounded = true;

//                    if (!e->entityHasComponent(my_ent, FLN_TYPE))
//                        continue;

//                    TypeData* type = getComponentData<TypeData>(e, my_ent, FLN_TYPE);
                    ent_type_t type = getType(e, my_ent);
                    if (type != 0 && m_typeToResponse[type] != nullptr) {
                        m_typeToResponse[type](e, my_ent, 0, true);
                    }
                }

//                if (physDat->grounded == false) {
//                    Transform groundTest = *getTransform(e, my_ent);
//                    PhysicsData phys_ = *physDat;
//                    groundTest.pos.y -= .1f;
//                    if (AABBtoAABBIntersect(&groundTest, &phys_, &trans, nullptr, false)) {
//                        if (trans.pos.y < getTransform(e, my_ent)->pos.y)
//                            physDat->grounded = true;
//                    }

//                }

            }
        }

        // DYNAMIC OBJECT COLLISIONS
        // optimize later by only going to highest value
        for (int ent = 0; ent < MAX_ENTITY; ent++) {


            if (!e->entityExists(ent) || ent == my_ent
                || !e->entityHasComponent(ent, FLN_COLLISION)
                || !e->entityHasComponent(ent, FLN_TRANSFORM)
                || checkOccured(my_ent, ent))
                continue;

            addOccured(my_ent, ent);

            // POTENTIAL ERROR:
            // offsets are not equal between both people, so might be weirdness based on ordering of entities
            // So for instance, because only "my_ent" is changing in function, lower numbered entities
            // will in theory be pushed around and not vice versa

            CollisionData* col = getComponentData<CollisionData>(e, my_ent, FLN_COLLISION);
            CollisionData* other_col = getComponentData<CollisionData>(e, ent, FLN_COLLISION);

            // OF NOTE: collision logic will be used for both entities because everything cycled through
            if (AABBtoAABBIntersect(getTransform(e, my_ent), physDat, getTransform(e, ent), nullptr, (col->col_type > 0 && other_col->col_type > 0))) {
//                std::cout << "dyn intersect " << (int)my_ent << " " << (int)ent << " c type " <<  (int)(col->col_type) << std::endl;
//                e->queueDestroyEntity(my_ent);
                //          IF type registered
                //              run collision logic on each entity for the registered type,
                //              pass in both entity id
                if (!e->entityHasComponent(my_ent, FLN_TYPE))
                    continue;

                TypeData* type = getComponentData<TypeData>(e, my_ent, FLN_TYPE);
                if (type != nullptr && m_typeToResponse[type->type] != nullptr) {
                    m_typeToResponse[type->type](e, my_ent, ent, false);
                }
            }

        }

    }
}

bool Physics::AABBtoAABBIntersect(Transform* transform, PhysicsData* physics, Transform* otherTransform, PhysicsData* otherPhysics, bool slide) {

//    PhysicsData* physDat = static_cast<PhysicsData*>(e->getComponentData(ent, FLN_PHYSICS));
//    Transform* transform = static_cast<Transform*>(e->getComponentData(ent, FLN_TRANSFORM));

//    PhysicsData* otherPhysDat = static_cast<PhysicsData*>(e->getComponentData(other_ent, FLN_PHYSICS));
//    Transform* otherTransform = static_cast<Transform*>(e->getComponentData(other_ent, FLN_TRANSFORM));

    // Assuming position is middle of primitive
    float ent_1_xmin = transform->pos.x - transform->scale.x/2;
    float ent_1_xmax = transform->pos.x + transform->scale.x/2;

    float ent_1_ymin = transform->pos.y - transform->scale.y/2;
    float ent_1_ymax = transform->pos.y + transform->scale.y/2;

    float ent_1_zmin = transform->pos.z - transform->scale.z/2;
    float ent_1_zmax = transform->pos.z + transform->scale.z/2;

    float ent_2_xmin = otherTransform->pos.x - otherTransform->scale.x/2;
    float ent_2_xmax = otherTransform->pos.x + otherTransform->scale.x/2;

    float ent_2_ymin = otherTransform->pos.y - otherTransform->scale.y/2;
    float ent_2_ymax = otherTransform->pos.y + otherTransform->scale.y/2;

    float ent_2_zmin = otherTransform->pos.z - otherTransform->scale.z/2;
    float ent_2_zmax = otherTransform->pos.z + otherTransform->scale.z/2;



    // Check for no overlap along each axis
    if (ent_1_xmax < ent_2_xmin || ent_1_xmin > ent_2_xmax ||
        ent_1_ymax < ent_2_ymin || ent_1_ymin > ent_2_ymax ||
        ent_1_zmax < ent_2_zmin || ent_1_zmin > ent_2_zmax) {
        return false; // No overlap
    }

    if (!slide)
        return true;

    // TEST THIS
    // What if the incoming velocity doesn't correspond to the smallest overlap?
    float overlap_x = std::max(std::min(ent_2_xmax, ent_1_xmax) - std::max(ent_2_xmin, ent_1_xmin), 0.0f);
    float overlap_y = std::max(std::min(ent_2_ymax, ent_1_ymax) - std::max(ent_2_ymin, ent_1_ymin), 0.0f);
    float overlap_z = std::max(std::min(ent_2_zmax, ent_1_zmax) - std::max(ent_2_zmin, ent_1_zmin), 0.0f);

    float smallest = std::min(overlap_x, std::min(overlap_y, overlap_z));



    if (smallest == overlap_x) {
        transform->pos.x -= copysign(overlap_x, physics->vel.x);
        physics->vel.x = 0;
    }
    else if (smallest == overlap_y) {
        transform->pos.y -= copysign(overlap_y, physics->vel.y);

        if (physics->vel.y <= 0)
            physics->grounded = true;

        physics->vel.y = 0;
    }
    else if (smallest == overlap_z) {
        transform->pos.z -= copysign(overlap_z, physics->vel.z);
        physics->vel.z = 0;
    }
    return true;
}

