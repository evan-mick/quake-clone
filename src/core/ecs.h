#ifndef ECS_H
#define ECS_H

#include <vector>
#include <chrono>
#include <array>
#include <functional>


typedef unsigned int flags_t;
typedef unsigned char entity_t;
typedef unsigned char entityType_t;
//typedef void (*system_t)(struct ECS*, entity_t entity_id, float delta_seconds);
typedef std::function<void(struct ECS*, entity_t, float)> system_t;
typedef std::function<void(entity_t)> entbroadcast_t;
//template <typename T> T* getComponentData(int entity_id, int flag_num);

// IN THEORY, these expressions, and any of the functions, should not have to be edited
// if you need more space for components or entity ID's, just change the underlying typedef above
constexpr entity_t MAX_ENT_VAL = -1;
constexpr size_t MAX_ENTITY = MAX_ENT_VAL + 1; // MAX VALUE OF ENTITY_T
constexpr int MAX_COMPONENTS = sizeof(flags_t) * 8;
// Infinite systems should be possible, but leaving it fixed
constexpr int MAX_SYSTEMS = 32;




class ECS
{
public:
    ECS();

    // Updates all the system for all objects
    void update();

    // Queues an entity with id [entity_id] to be destroyed at the end of the current update
    void queueDestroyEntity(entity_t entity_id);

    // Creates an entity
    // Input a bitwise or representation of the component flags
    // Returns the id on success, -1 on failure
    entity_t createEntityWithBitFlags(flags_t bitwise_flags);

    // Creates an entity
    // Input a bunch of flag numbers
    entity_t createEntity(std::initializer_list<int> flag_numbers);
//    entity_t createEntity(entityType_t type, std::initializer_list<int> flag_numbers);

    // Returns the raw bitmask of the inputted entity
    flags_t getEntityBitMask(entity_t entity_id);

    // Registers a system in the ECS
    // Will run the function [system_function] on all entities with [required_flags]
    void registerSystemWithBitFlags(system_t system_function, flags_t required_flags);

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
    void* getComponentData(entity_t entity_id, int flag_num);
//    template <typename T> T* getComponentData(int entity_id, int flag_num);

    // Returns the # of bytes written to the char* array
    // Sets the buff_ptr to a heap allocated pointer to the buffer
    int serializeData(char** buff_ptr);

    // ASSUMPTIONS serialized_data is a well-formed, serialization of game state
    // ignore is an MAX_ENTITY sized array of entity -> bool to not add to the data
    // IMPORTANT: some functionality incomplete, especially for objects that didn't exist before or got destroyed
    void deserializeIntoData(char* serialized_data, size_t max_size, const bool* ignore);


    // Does the entity have the component with the given flag?
    inline bool entityHasComponent(entity_t ent, int flag_num) {
        return m_entities[ent] | (1 << flag_num);
    }

    // Check if an entity exists
    inline bool entityExists(entity_t ent) {
        return (m_entities[ent] != 0);
    }

    inline bool isComponentRegistered(int flag_num) {
        return m_component_registered[flag_num];
    }

    inline void addBroadcast(entbroadcast_t broadcast) {
        m_onCreateEntityBroadcast.push_back(broadcast);
    }

    inline float getRecentDelta() {
        return m_deltaTime;
    }



private:

    struct SystemData {
        system_t func;
        flags_t req_flags;
    };

    // Entity id -> bitmask flags
    std::array<flags_t, MAX_ENTITY> m_entities{};

    // Component id -> component data buffer
    std::array<void*, MAX_COMPONENTS> m_components{};
    // Component id -> bool is the component registered
    std::array<bool, MAX_COMPONENTS> m_component_registered{};
    // Component id -> component struct size
    std::array<size_t, MAX_COMPONENTS> m_component_num_to_size{};

    std::vector<SystemData> m_systems = {};

    std::vector<entbroadcast_t> m_onCreateEntityBroadcast = {};
    inline void doBroadcast(std::vector<entbroadcast_t>& funcs, entity_t ent) {
        for (entbroadcast_t& func : funcs) {
            func(ent);
        }
    }

    // Amount of data stored, needed for serialization and deserialization
    // BE ON THE LOOKOUT FOR BUGS RELATED TO THIS
    size_t m_usedDataSize = 0;

    size_t m_nextUnallocEntity = 0;

    float m_deltaTime;

    // Destruction
    std::vector<int> m_destroyQueue = {};
    void destroyQueuedEntities();
    void destroyEntity(entity_t entity_id);


    // For delta time calculation
    std::chrono::time_point<std::chrono::steady_clock> m_lastUpdate;


};

#endif // ECS_H
