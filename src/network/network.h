#ifndef NETWORK_H
#define NETWORK_H

#include "../core/ecs.h"


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

const u_int16_t default_port = 42069; // hell yeah
const int MAX_PLAYERS = 4;

struct Connection {
    sockaddr_in sock_data;
    long last_rec_tick;
    int socket;
    int entity; // if the other side is the server, -1, otherwise keep entity_id of the player
};


class Network
{
public:
    Network(bool server);


private:
    bool m_isServer = false;

    // To store what entities they have authority over
    int m_hasAuthority[MAX_ENTITY] = {};

    Connection m_connections[MAX_PLAYERS] = {};
};

#endif // NETWORK_H
