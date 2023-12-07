#include "network.h"


Network::Network(bool server, ECS* ecs)
{
    m_isServer = server;
    m_ecs = ecs;

    // Server has authority over everything by default
    if (server) {
        std::fill(m_hasAuthority.begin(), m_hasAuthority.end(), true);
    }
    m_listenThread = std::thread([this]() { this->listenThread(); });
}



void Network::listenThread() {

    // Constantly listen for receiving
    // May need to add stuff to check for when no longer listening

    char* buffer[1400] {};
//    recv()

}

Gamestate* Network::popLeastRecentGamestate() {
    return nullptr;

}

void Network::deserializeAllDataIntoECS(ECS* ecs) {
    // TODO: tick checks, authority check (need to get client id)
    Gamestate* curState = popLeastRecentGamestate();
    while (curState != nullptr) {
        ecs->deserializeIntoData(curState->data, curState->data_size, nullptr);
    }
}

void Network::connect(const char* ip, const char* port) {

    //sockaddr_in in;

}

void Network::shutdown() {
    m_shutdown = true;
    m_listenThread.join();
}


int Network::setupUDPConn(const char* address, const char* port, addrinfo* info) {
    int rv;
    int sock;
//    std::cout << "starting udp connection on: " << address << ":" << port << std::endl;

    // Ask for a socket that listens on all addresses
    struct addrinfo hints, *res, *servinfo;
    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_INET;       // Request an IPv4 socket
    hints.ai_socktype = SOCK_DGRAM;  // UDP socket
    hints.ai_flags = AI_PASSIVE;     // Bind to all addresses on the system

    if ((rv = getaddrinfo(address, port, &hints, &servinfo)) != 0) {
        perror("getaddrinfo");
        return -1;
    }

    // Look at all the results and bind to the first one
    // (Technically, we should be able to eliminate this loop, since we only picked AF_INET)
    for (res = servinfo; res != NULL; res = res->ai_next) {
        if((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            continue;
        }
        break;
    }

    if (res == nullptr) {
        printf("Could not bind to socket\n");
        return -1;
    }

    *info = *res;

    return sock;
}
