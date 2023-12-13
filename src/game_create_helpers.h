#ifndef GAME_CREATE_HELPERS_H
#define GAME_CREATE_HELPERS_H

#include "game_types.h"
#include "scene/scenedata.h"
#include "glm/gtx/transform.hpp"
#include <random>

const int PLAYERSIZE = sizeof(entity_t) + sizeof(flags_t) + sizeof(Transform) + sizeof(PhysicsData) + sizeof(Test) + sizeof(Renderable) + sizeof(InputData) + sizeof(CollisionData);

const int PROJSIZE = sizeof(entity_t) + sizeof(flags_t) + sizeof(Transform) + sizeof(PhysicsData) + sizeof(CollisionData);


inline int cur_player_color = 0;

inline entity_t createPlayer(ECS* e, glm::vec3 pos) {
    entity_t ent = e->createEntity({ FLN_TRANSFORM, FLN_PHYSICS, FLN_TEST, FLN_RENDER, FLN_INPUT, FLN_COLLISION, FLN_TYPE, FLN_HEALTH, FLN_PLAYERINFO });

    if (e->isComponentRegistered(FLN_RENDER)) {
        Renderable* rend = static_cast<Renderable*>(e->getComponentData(ent, FLN_RENDER));
        //    rend->model_id = static_cast<uint8_t>(PrimitiveType::PRIMITIVE_SPHERE);
        rend->color = (cur_player_color % 5) + 2;
        cur_player_color++;
        rend->model_id = 5;
    }

    trySetType(e, ent, ET_PLAYER);

    Transform* trans = static_cast<Transform*>(e->getComponentData(ent, FLN_TRANSFORM));
    trans->pos = pos;
    trans->scale = glm::vec3(1, 2, 1);

    Health* hlth = static_cast<Health*>(e->getComponentData(ent, FLN_HEALTH));
    hlth->amt = MAX_HEALTH;

    CollisionData* col = getComponentData<CollisionData>(e, ent, FLN_COLLISION);
    col->col_type = 1;
    return ent;
}


inline void respawnPlayer(ECS* e, entity_t ent) {
    getTransform(e, ent)->pos = glm::vec3(0, 50.f, 0);
    getPhys(e, ent)->vel = glm::vec3(0, 0.f, 0);
    getPhys(e, ent)->accel = glm::vec3(0, 0.f, 0);

    getComponentData<Health>(e, ent, FLN_HEALTH)->amt = MAX_HEALTH;
    getComponentData<PlayerInfo>(e, ent, FLN_PLAYERINFO)->shotCooldown = 0.f;
}


inline entity_t createProjectile(ECS* e, glm::vec3 pos, glm::vec3 move) {
    entity_t proj = e->createEntity({FLN_TRANSFORM, FLN_PHYSICS, FLN_RENDER, FLN_COLLISION, FLN_TYPE, FLN_DESTROYTIME, FLN_PROJECTILE });
    getTransform(e, proj)->pos = pos+glm::vec3(0,2.2,0);
    getTransform(e, proj)->scale = glm::vec3(.5f, .5f, .5f);

    if (e->isComponentRegistered(FLN_RENDER)) {
        Renderable* rend = static_cast<Renderable*>(e->getComponentData(proj, FLN_RENDER));
        rend->model_id = static_cast<uint8_t>(PrimitiveType::PRIMITIVE_SPHERE);
        rend->color = 4;
    }

    trySetType(e, proj, ET_PROJ);

    CollisionData* col = getComponentData<CollisionData>(e, proj, FLN_COLLISION);
    col->col_type = -1;

    DestroyData* dat = getComponentData<DestroyData>(e, proj, FLN_DESTROYTIME);
    dat->timer = 5.f;

//    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rot.x, glm::vec3(0.0f, 1.0f, 0.0f));
//    rotationMatrix = glm::rotate(rotationMatrix, rot.y, glm::vec3(0.0f, 0.0f, 1.0f));
    getPhys(e, proj)->vel = move * 40.f;
    return proj;
}


inline entity_t createExplosion(ECS* e, glm::vec3 pos) {
    entity_t proj = e->createEntity({FLN_TRANSFORM, FLN_PHYSICS, FLN_RENDER, FLN_COLLISION, FLN_TYPE, FLN_DESTROYTIME, FLN_PROJECTILE });
    getTransform(e, proj)->pos = pos;
    getTransform(e, proj)->scale = glm::vec3(10.f, 10.f, 10.f);

    if (e->isComponentRegistered(FLN_RENDER)) {
        Renderable* rend = static_cast<Renderable*>(e->getComponentData(proj, FLN_RENDER));
        rend->model_id = static_cast<uint8_t>(PrimitiveType::PRIMITIVE_SPHERE);
        rend->color = 7;
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
