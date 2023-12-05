#ifndef NETWORK_H
#define NETWORK_H

#include "../core/ecs.h"


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <queue>


const u_int16_t default_port = 42069; // hell yeah
const int MAX_PLAYERS = 4;



struct Gamestate {
    int tick;
    size_t data_size;
    char* data;

    // for comparisons
    bool operator()(const Gamestate& l, const Gamestate& r) const { return l.tick > r.tick; };
};

struct Connection {
    sockaddr_in sock_data;
    long last_rec_tick;
    int socket;
    int entity; // if the other side is the server, -1, otherwise keep entity_id of the player
};

//bool compare(Gamestate a, Gamestate b) {
//    return (a.tick < b.tick);
//}


class Network
{
public:
    Network(bool server, ECS* ecs);




private:
    bool m_isServer = false;
    ECS* m_ecs;

    // ONLY RELEVENT TO CLIENTS AS OF NOW
    // what should server be receiving? should this be in connection?
    std::priority_queue<Gamestate, std::vector<Gamestate>, Gamestate> m_recentGamestates;


    // To store what entities they have authority over
    std::array<int, MAX_ENTITY> m_hasAuthority{};

    std::array<Connection, MAX_PLAYERS> m_connections{};

    void listenForData();
};

#endif // NETWORK_H
