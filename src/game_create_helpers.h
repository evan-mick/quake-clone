#ifndef GAME_CREATE_HELPERS_H
#define GAME_CREATE_HELPERS_H

#include "game_types.h"
#include "scene/scenedata.h"
#include "glm/gtx/transform.hpp"

const int PLAYERSIZE = sizeof(entity_t) + sizeof(flags_t) + sizeof(Transform) + sizeof(PhysicsData) + sizeof(Test) + sizeof(Renderable) + sizeof(InputData) + sizeof(CollisionData);

const int PROJSIZE = sizeof(entity_t) + sizeof(flags_t) + sizeof(Transform) + sizeof(PhysicsData) + sizeof(CollisionData);


inline entity_t createPlayer(ECS* e, glm::vec3 pos) {
    entity_t ent = e->createEntity({ FLN_TRANSFORM, FLN_PHYSICS, FLN_TEST, FLN_RENDER, FLN_INPUT, FLN_COLLISION, FLN_TYPE, FLN_HEALTH });

    if (e->isComponentRegistered(FLN_RENDER)) {
        Renderable* rend = static_cast<Renderable*>(e->getComponentData(ent, FLN_RENDER));
        //    rend->model_id = static_cast<uint8_t>(PrimitiveType::PRIMITIVE_SPHERE);
        rend->model_id = 5;
    }

    trySetType(e, ent, ET_PLAYER);

    Transform* trans = static_cast<Transform*>(e->getComponentData(ent, FLN_TRANSFORM));
    trans->pos = pos;
    trans->scale = glm::vec3(1, 2, 1);

    CollisionData* col = getComponentData<CollisionData>(e, ent, FLN_COLLISION);
    col->col_type = 1;
    return ent;
}



inline entity_t createProjectile(ECS* e, glm::vec3 pos, glm::vec3 move) {
    entity_t proj = e->createEntity({FLN_TRANSFORM, FLN_PHYSICS, FLN_RENDER, FLN_COLLISION, FLN_TYPE, FLN_DESTROYTIME, FLN_SHOTFROM });
    getTransform(e, proj)->pos = pos+glm::vec3(0,2.2,0);
    getTransform(e, proj)->scale = glm::vec3(.5f, .5f, .5f);

    if (e->isComponentRegistered(FLN_RENDER)) {
        Renderable* rend = static_cast<Renderable*>(e->getComponentData(proj, FLN_RENDER));
        rend->model_id = static_cast<uint8_t>(PrimitiveType::PRIMITIVE_SPHERE);
    }

    trySetType(e, proj, ET_PROJ);

    CollisionData* col = getComponentData<CollisionData>(e, proj, FLN_COLLISION);
    col->col_type = -1;

    DestroyData* dat = getComponentData<DestroyData>(e, proj, FLN_DESTROYTIME);
    dat->timer = 5.f;

//    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rot.x, glm::vec3(0.0f, 1.0f, 0.0f));
//    rotationMatrix = glm::rotate(rotationMatrix, rot.y, glm::vec3(0.0f, 0.0f, 1.0f));
    getPhys(e, proj)->vel = move * 20.f;
    return proj;
}


inline entity_t createExplosion(ECS* e, glm::vec3 pos) {
    entity_t proj = e->createEntity({FLN_TRANSFORM, FLN_PHYSICS, FLN_RENDER, FLN_COLLISION, FLN_TYPE, FLN_DESTROYTIME });
    getTransform(e, proj)->pos = pos;
    getTransform(e, proj)->scale = glm::vec3(10.f, 10.f, 10.f);

    if (e->isComponentRegistered(FLN_RENDER)) {
        Renderable* rend = static_cast<Renderable*>(e->getComponentData(proj, FLN_RENDER));
        rend->model_id = static_cast<uint8_t>(PrimitiveType::PRIMITIVE_SPHERE);
    }

    trySetType(e, proj, ET_EXPLOSION);

    CollisionData* col = getComponentData<CollisionData>(e, proj, FLN_COLLISION);
    col->col_type = -1;

    DestroyData* dat = getComponentData<DestroyData>(e, proj, FLN_DESTROYTIME);
    dat->timer = 1.f;

    getPhys(e, proj)->vel = glm::vec3(0, 0, 0);
    getPhys(e, proj)->accel = glm::vec3(0, 0, 0);
    return proj;
}
#endif // GAME_CREATE_HELPERS_H
