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

#include <sys/socket.h>
#include <netdb.h>



const uint16_t default_port = 42069; // hell yeah
const int MAX_PLAYERS = 4;

#pragma pack(1)
struct Packet {
    unsigned int tick;
    char command;
    char* data;
};

struct TickData {
    unsigned int tick;
    char* data;
};

struct TickBuffer {
    std::mutex mutex;
    std::queue<TickData> buffer;
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
    long last_rec_tick;
    int socket;
    uint16_t port;
    uint32_t ip;
    int entity; // if the other side is the server, -1, otherwise keep entity_id of the player
    TickBuffer tick_buffer;
};

//bool compare(Gamestate a, Gamestate b) {
//    return (a.tick < b.tick);
//}



class Network
{
public:
    Network(bool server, ECS* ecs, const char* ip, const char* port);

    int connect(const char* ip, const char* port);
    Connection* getConnection(uint32_t ip);
    Gamestate* popLeastRecentGamestate();
    void deserializeAllDataIntoECS(ECS* ecs);

    void shutdown();
    void broadcastClientGS(ECS* ecs, Connection* conn, int tick);
    void addConnection(uint32_t ip, Connection* conn);
    void editConnection(uint32_t ip);
    
    void onTick();
    int initClient(const char* ip, const char* port);
    void updateTickBuffer(Packet packet, Connection* conn, unsigned int tick);
    

private:
    bool m_isServer = false;
    ECS* m_ecs;
    std::atomic_bool m_shutdown; // equal sign needs to be removed and defined in the consructor I think
    std::mutex m_connectionMutex;
    // ONLY RELEVENT TO CLIENTS AS OF NOW
    // what should server be receiving? should this be in connection?
    std::priority_queue<Gamestate, std::vector<Gamestate>, Gamestate> m_recentGamestates;

    // To store what entities they have authority over
    std::array<int, MAX_ENTITY> m_hasAuthority{};

    std::array<Connection, MAX_PLAYERS> m_connections{}; // not using this atm
    std::unordered_map<uint32_t, Connection*> m_connectionMap{};
//    void listenForData();

    std::thread m_listenThread;
    
    void serverListen(const char* ip, const char* port);
    void clientListen();

    int setupUDPConn(const char* address, const char* port);

    // Tick stuff
    Timer m_timer = Timer(1/20.f);
    float m_tickRate = 1/20.f;
};

#endif // NETWORK_H
