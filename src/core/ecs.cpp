#include "ecs.h"
#include <cstring>
#include <iostream>
#include <array>

ECS::ECS()
{
    m_lastUpdate = std::chrono::steady_clock::now();
}

void ECS::update() {
    // TODO: delta
    std::chrono::time_point<std::chrono::steady_clock>  now = std::chrono::steady_clock::now();
    m_deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastUpdate).count() / 1000000.0f;
    m_lastUpdate = now;

    for (SystemData& data : m_systems) {
        // POTENTIAL OPTIMIZATION: Only cycle up to furthest entity
        for (size_t ent = 0; ent < MAX_ENTITY; ent++) {
            flags_t flags = m_entities[ent];
            if ((flags & data.req_flags) == data.req_flags)
                data.func(this, (entity_t)ent, m_deltaTime);
        }
    }
    destroyQueuedEntities();
}

void ECS::queueDestroyEntity(entity_t entity_id) {
    m_destroyQueue.push_back(entity_id);
}

flags_t ECS::getEntityBitMask(entity_t entity_id) {
//    if (entity_id < 0 || entity_id >= MAX_ENTITY)
//        return 0;
    return m_entities[entity_id];
}

entity_t ECS::createEntityWithBitFlags(flags_t flags) {

    if (m_nextUnallocEntity >= MAX_ENTITY)
        return -1;

    std::cout << m_nextUnallocEntity << " next pre " << std::endl;

    // Register entity
    entity_t ent_id = m_nextUnallocEntity;
    m_entities[ent_id] = flags;

    // Add used data
    for (int flag = 0; flag < MAX_COMPONENTS; flag++) {
        if ((m_entities[ent_id] & (1 << flag)) && m_component_registered[flag]) {
            m_usedDataSize += m_component_num_to_size[flag];
        }
    }

    // Setup next entity to allocate
    do {
        m_nextUnallocEntity++;
    }
    while (m_nextUnallocEntity < MAX_ENTITY && m_entities[m_nextUnallocEntity] != 0);

    std::cout << m_nextUnallocEntity << " next post " << std::endl;
    doBroadcast(m_onCreateEntityBroadcast, ent_id);

    return ent_id;
}

entity_t ECS::createEntity(std::initializer_list<int> flag_numbers) {
    int input_flag = 0;

    for (int flag : flag_numbers) {
        if (m_component_registered[flag])
            input_flag = input_flag | (1 << flag);
    }


    return createEntityWithBitFlags(input_flag);

}

void ECS::destroyEntity(entity_t id) {

    // Take away used data
    for (int flag = 0; flag < MAX_COMPONENTS; flag++) {
        if ((m_entities[id] & (1 << flag)) && m_component_registered[flag]) {
            m_usedDataSize -= m_component_num_to_size[flag];
        }
    }

    m_entities[id] = 0;
    m_nextUnallocEntity = std::min((size_t)id, m_nextUnallocEntity);
}

void ECS::destroyQueuedEntities() {
    for (int ent : m_destroyQueue) {
         destroyEntity(ent);
    }
    m_destroyQueue.clear();
}

void ECS::registerSystemWithBitFlags(system_t system_function, flags_t required_flags) {
    SystemData dat = {};
    dat.func = system_function;
    dat.req_flags = required_flags;

    m_systems.push_back(dat);
}

void ECS::registerSystem(system_t system_function, std::initializer_list<int> flag_numbers) {
    flags_t input_flag = 0;
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
void* ECS::getComponentData(entity_t entity_id, int flag_num) {

    flags_t flag = (1 << (flag_num));
    bool equal = ((m_entities[entity_id] & flag) == flag);
    std::cout << "m_entities[id]: " << std::to_string(m_entities[entity_id])
              << " m_entities[entity_id] & flag: " << std::to_string(m_entities[entity_id] & flag)
              << " flag: " << std::to_string(flag)
              << " equal: " << std::to_string(equal) <<std::endl;
//    bool ent_in_bounds = (entity_id < MAX_ENTITY && entity_id >= 0); this already checked by virtue of entity_t
    bool flag_in_bounds = (flag_num >= 0 || flag_num < MAX_COMPONENTS);

    if (!flag_in_bounds || !m_component_registered[flag_num] ){
        //|| !equal /*|| !ent_in_bounds*/) {

        if (!flag_in_bounds)
            std::cout << "!flag_in_bounds" << std::endl;
        if (!m_component_registered[flag_num])
            std::cout << "!m_component_registered[flag_num]" << std::endl;
        if (!equal)
            std::cout << "!equal" << std::endl;
        return nullptr;
    }
    return ((char*)(m_components[flag_num])) + (m_component_num_to_size[flag_num] * entity_id);
}


int ECS::serializeData(char** buff_ptr) {
    *buff_ptr = new char[m_usedDataSize];

    size_t ob_ptr = 0;
    // size_t used = 0;
    for (int ent = 0; ent < MAX_ENTITY; ent++) {

        if (m_entities[ent] == 0)
            continue;

        // Copy entity id and flags
        *((entity_t*)(*buff_ptr + ob_ptr)) = ent;
        ob_ptr += sizeof(entity_t);

        *((flags_t*)(*buff_ptr + ob_ptr)) = m_entities[ent];
        ob_ptr += sizeof(flags_t);

        // Copy component data
        for (int com = 0; com < MAX_COMPONENTS; com++) {
            bool has_flag = m_entities[ent] & (1 << com);
            if (!m_component_registered[com] || !has_flag)
                continue;

            memcpy((*buff_ptr + ob_ptr), getComponentData(ent, com), m_component_num_to_size[com]);
            ob_ptr += m_component_num_to_size[com];
        }
    }


    return ob_ptr;//..m_usedDataSize;
}

void ECS::deserializeIntoData(char* serialized_data, size_t max_size, const bool* ignore) {
    // IMPORTANT: what happens to used data size when a new object is deserialized in?
    // ALSO, ensure that tip of data isn't full if flags are empty/object is destroyed
    std::cout << "Deserializing into data (ECS)" << std::endl;

    if (serialized_data == nullptr) {
        std::cout << "Null Serialized Data" << std::endl;
        return;
    }
    size_t ob_ptr = 0;

    entity_t ent;
    flags_t flags;

    while (ob_ptr < max_size) {

        // get initial entity data
        memcpy(&ent, &serialized_data[ob_ptr], sizeof(entity_t));
        ob_ptr += sizeof(entity_t);
        memcpy(&flags, &serialized_data[ob_ptr], sizeof(flags_t));
        ob_ptr += sizeof(flags_t);

        // ignore entities on the ignore list
        if (ignore != nullptr && ignore[ent])
            continue;

        // end early if empty entity / end of data
        if (ent == 0 && flags == 0) {
            std::cout << "breaking"  <<std::endl;
            break;
        }


        // Copy over all component data
        // IMPORTANT: what if entity doesn't exist before?
        m_entities[ent] = flags;
        for (int com = 0; com < MAX_COMPONENTS; com++) {
            bool has_flag = m_entities[ent] & (1 << com);
            if (!m_component_registered[com] || !has_flag) {
                continue;
            }

            std::cout << "found component to update" << std::endl;

            memcpy(getComponentData(ent, com), (serialized_data + ob_ptr), m_component_num_to_size[com]);
            ob_ptr += m_component_num_to_size[com];

        }
    }
}
