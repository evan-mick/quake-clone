#ifndef ECS_H
#define ECS_H

#include <vector>
#include <chrono>

const int MAX_ENTITY = 256;
const int MAX_COMPONENTS = 32;
// In theory, infinite systems should be possible, but leaving it fixed
const int MAX_SYSTEMS = 32;


typedef int flags_t;
typedef char entity_t;
typedef void (*system_t)(struct ECS*, int entity_id, float delta_seconds);
template <typename T> T* getComponentData(int entity_id, int flag_num);

class ECS
{
public:
    ECS();

    // Updates all the system for all objects
    void update();

    // Queues an entity with id [entity_id] to be destroyed at the end of the current update
    void queueDestroyEntity(int entity_id);

    // Creates an entity
    // Input a bitwise or representation of the component flags
    // Returns the id on success, -1 on failure
    int createEntityWithBitFlags(int flags);

    // Creates an entity
    // Input a bunch of flag numbers
    int createEntity(std::initializer_list<int> flag_numbers);

    // Returns the raw bitmask of the inputted entity
    int getEntityBitMask(int entity_id);

    // Registers a system in the ECS
    // Will run the function [system_function] on all entities with [required_flags]
    void registerSystemWithBitFlags(system_t system_function, int required_flags);

    // Registers a system in the ECS
    // Will run the function [system function] on all entities with the given flag numbers
    void registerSystem(system_t system_function, std::initializer_list<int> flag_numbers);


    // Registers a component with the given type
    // The flag num is a number 0 <= flag_num < sizeof(flags_t)
    // Returns the bitmask if it succeedes, returns -1 if it fails
    int registerComponent(int flag_num, size_t data_size);

//    template <typename T> int registerComponent(int flag_num);

    // Gets a reference to the component data
    // Pass in the type of struct
    // OF NOTE: tried to avoid "void*" and to use templates, but had a 2 hour time sink issue trying to make it work
    // Just static cast pls :'), we know what it'll be b/c of the flag num
    void* getComponentData(int entity_id, int flag_num);
//    template <typename T> T* getComponentData(int entity_id, int flag_num);



private:

    struct SystemData {
        system_t func;
        int req_flags;
    };

    int m_entities[MAX_ENTITY] = {};

    void* m_components[MAX_COMPONENTS] = {};
    bool m_component_registered[MAX_COMPONENTS] = {};
    size_t m_component_num_to_size[MAX_COMPONENTS] = {};

    std::vector<SystemData> m_systems = {};



//    int m_oneAfterLastEntity = 0;
    int m_nextUnallocEntity = 0;

    std::vector<int> m_destroyQueue = {};

    void destroyQueuedEntities();
    void destroyEntity(int entity_id);


    std::chrono::time_point<std::chrono::steady_clock> m_lastUpdate;


};

#endif // ECS_H
