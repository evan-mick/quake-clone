#include "ecs.h"
#include <iostream>


ECS::ECS()
{
    m_lastUpdate = std::chrono::steady_clock::now();

}



void ECS::update() {
    // TODO: delta
    std::chrono::time_point<std::chrono::steady_clock>  now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastUpdate).count() / 1000000.0f;
    m_lastUpdate = now;

    for (SystemData& data : m_systems) {
        // POTENTIAL OPTIMIZATION: Only cycle up to furthest entity
        for (int ent = 0; ent < MAX_ENTITY; ent++) {
            int flags = m_entities[ent];
            if ((flags & data.req_flags) == data.req_flags)
                data.func(this, ent, deltaTime);
        }
    }
    destroyQueuedEntities();
}

void ECS::queueDestroyEntity(int entity_id) {
    m_destroyQueue.push_back(entity_id);
}

int ECS::getEntityBitMask(int entity_id) {
    if (entity_id < 0 || entity_id >= MAX_ENTITY)
        return 0;
    return m_entities[entity_id];
}

int ECS::createEntityWithBitFlags(int flags) {

    if (m_nextUnallocEntity >= MAX_ENTITY)
        return -1;

    int ent_id = m_nextUnallocEntity;
    m_entities[ent_id] = flags;

    do {
        m_nextUnallocEntity++;
    }
    while (m_nextUnallocEntity < MAX_ENTITY && m_entities[m_nextUnallocEntity] != 0);

    return ent_id;
}

int ECS::createEntity(std::initializer_list<int> flag_numbers) {
    int input_flag = 0;

    for (int flag : flag_numbers) {
        input_flag = input_flag | (1 << flag);
    }

    return createEntityWithBitFlags(input_flag);

}

void ECS::destroyEntity(int id) {
    m_entities[id] = 0;
    m_nextUnallocEntity = std::min(id, m_nextUnallocEntity);
}

void ECS::destroyQueuedEntities() {
    for (int ent : m_destroyQueue) {
         destroyEntity(ent);
    }
    m_destroyQueue.clear();
}

void ECS::registerSystemWithBitFlags(system_t system_function, int required_flags) {
    SystemData dat = {};
    dat.func = system_function;
    dat.req_flags = required_flags;

    m_systems.push_back(dat);
}

void ECS::registerSystem(system_t system_function, std::initializer_list<int> flag_numbers) {
    int input_flag = 0;
    for (int flag : flag_numbers) {
        input_flag = input_flag | (1 << flag);
    }
    registerSystemWithBitFlags(system_function, input_flag);
}



//template <typename T>
int ECS::registerComponent(int flag_num, size_t data_size) {

    if (flag_num >= MAX_COMPONENTS || flag_num < 0) {
        return -1;
    }

    if (m_components[flag_num] != nullptr) {
        return -1;
    }

    m_components[flag_num] = new char[data_size][MAX_ENTITY];
    memset(m_components[flag_num], 0, data_size * MAX_ENTITY);

    m_component_registered[flag_num] = true;
    m_component_num_to_size[flag_num] = data_size;

    return 1 << flag_num;
}


//template <typename T>
void* ECS::getComponentData(int entity_id, int flag_num) {

    int flag = (1 << (flag_num));
    bool equal = ((m_entities[entity_id] & flag) == flag);
    bool ent_in_bounds = (entity_id < MAX_ENTITY && entity_id >= 0);
    bool flag_in_bounds = (flag_num >= 0 || flag_num < MAX_COMPONENTS);

    if (!flag_in_bounds || !m_component_registered[flag_num]
        || !equal || !ent_in_bounds)
        return nullptr;

    return ((char*)(m_components[flag_num])) + (m_component_num_to_size[flag_num] * entity_id);
}
