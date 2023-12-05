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

}

void Network::deserializeAllDataIntoECS(ECS* ecs) {
    // TODO: tick checks, authority check (need to get client id)
    Gamestate* curState = popLeastRecentGamestate();
    while (curState != nullptr) {
        ecs->deserializeIntoData(curState->data, curState->data_size, nullptr);
    }
}

void Network::connect(const char* ip, const char* port) {

    sockaddr_in in;

}

void Network::shutdown() {
    m_shutdown = true;
    m_listenThread.join();
}
