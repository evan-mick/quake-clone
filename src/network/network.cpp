#include "Network.h"

Network::Network(bool server, ECS* ecs)
{
    m_isServer = server;
    m_ecs = ecs;

    // Server has authority over everything by default
    if (server) {
        std::fill(m_hasAuthority.begin(), m_hasAuthority.end(), true);
    }
}

