#ifndef NETWORK_H
#define NETWORK_H

#include "../core/ecs.h"
#include "../core/timer.h"
#include <unordered_map>
#include <stdlib.h>
//#include <unistd.h>
#include <sys/types.h>
//#include <sys/socket.h>
#include <arpa/inet.h>
//#include <netinet/in.h>
#include <cstring>
#include <queue>
#include <thread>
#include <mutex>
#include "../game_types.h"
#include <sys/socket.h>
#include <netdb.h>

/*
 * Queue support (again)
 *
 * Multiple Packet support
 * - server sends multiple packets
 *      - find breakpoint thats at an entity
 *      - send a packet for each seg
 * - on client deserialize, check last time object was updated, if before incoming tick, update, otherwise, drop it
 *
 *
 */




extern const char* DEFAULT_PORT;
const int MAX_PLAYERS = 4;

#pragma pack(1)
struct Packet {
    unsigned int tick;
    char command;
    // char* data;
};





struct Gamestate {
    int tick;
    size_t data_size;
    char* data;

    // for comparisons, want the LEAST recent thing to be first
    // will iterate through all game states and set until its empty
    bool operator()(const Gamestate& l, const Gamestate& r) const { return l.tick < r.tick; };
};

struct Connection {
//    sockaddr sock_data;
    long last_rec_tick = 0;
    int socket;
    uint16_t port;
    uint32_t ip;
    entity_t entity; // if the other side is the server, -1, otherwise keep entity_id of the player
    std::chrono::steady_clock::time_point lastActivityTime;
    std::chrono::seconds timeoutDuration;

    // Constructor to initialize the connection
    Connection()  {
        last_rec_tick = 0;
        socket = 0;
        port = 0;
        ip = 0;
        entity = 0;
        timeoutDuration = std::chrono::seconds(5);
        resetTimer();
    }

    // Function to reset the timer
    void resetTimer() {
        lastActivityTime = std::chrono::steady_clock::now();
    }

    // Function to check if the connection has timed out
    bool hasTimedOut() const {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastActivityTime);
        return elapsed >= timeoutDuration;
    }
     float discon_timer = 5.f;
};

struct TickData {
     long tick;
     size_t data_size;
     Connection* from_conn;
     char* data;
};

//bool compare(Gamestate a, Gamestate b) {
//    return (a.tick < b.tick);
//}

const int FULL_PACKET = 1400;



class Network
{
public:
    Network(bool server, ECS* ecs, const char* ip);

    int connect(const char* ip, const char* port);
    Connection* getConnection(uint64_t ip);
    Gamestate* popLeastRecentGamestate();
    void deserializeAllDataIntoECS();
    void disconnectClient(Connection* conn);
    void shutdown();
    void broadcastGS(ECS* ecs, Connection* conn, int tick);
    void addConnection(uint64_t ipport, Connection* conn);
    void editConnection(Connection* conn, unsigned int tick);

    void onTick(unsigned int tick);
    // int initClient(const char* ip, const char* port);

    // Input a HEAP ALLOCATED BUFFER data, heap allocated connection, and a tick
    void updateTickBuffer(char* data, size_t data_size, Connection* conn, unsigned int tick);
    void broadcastOnTick(float delta);
//    void deserializeOnTick(float delta);
//    bool tickBufferReady();

    inline entity_t getMyPlayerEntityID() {
        return m_myPlayerEntityID;
    }

    inline void updateLastActivityTime(Connection* conn) {
        conn->lastActivityTime = std::chrono::steady_clock::now();
    }

    inline bool hasTimedOut(const Connection* conn) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - conn->lastActivityTime);
        return elapsed >= conn->timeoutDuration;
    }

//    inline void setAuthority(entity_t ent) {
//        m_hasAuthority[ent] = true;
//    }


private:

    std::mutex mutex_;
    std::queue<TickData> buffer;

    TickData next;

    entity_t m_myPlayerEntityID = 0;
    bool m_isServer = false;
    ECS* m_ecs;
    std::atomic_bool m_shutdown; // equal sign needs to be removed and defined in the consructor I think
    std::mutex m_connectionMutex;
    // ONLY RELEVENT TO CLIENTS AS OF NOW
    // what should server be receiving? should this be in connection?
//    std::priority_queue<Gamestate, std::vector<Gamestate>, Gamestate> m_recentGamestates;

    std::array<Connection, MAX_PLAYERS> m_connections{}; // not using this atm
    std::unordered_map<uint64_t, Connection*> m_connectionMap{};
//    void listenForData();
    std::thread m_listenThread;

    void serverListen(const char* ip, const char* port);
    void clientListen();

    int setupUDPConn(const char* address, const char* port, bool bind_sock, addrinfo** outinfo);

    // Tick stuff
    Timer m_timer = Timer(TICK_RATE);
    float m_tickRate = TICK_RATE;

    inline uint64_t ipport(uint32_t ip, uint16_t port) {
        uint64_t return_val = port;
        return_val = return_val << 16;
        return_val |= ip;
        return return_val;
    }
};

#endif // NETWORK_H
