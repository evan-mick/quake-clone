#include "network.h"
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include "../game_types.h"

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
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;

    int serverSocket = setupUDPConn(NULL, "42069"); // NULL for localhost
    if (serverSocket < 0) {
        // Handle error: unable to set up UDP connection
        return;
    }

    while (!m_shutdown) {
        Packet packet;
        struct sockaddr clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        int bytesReceived = recvfrom(serverSocket, (char*)&packet, sizeof(packet), 0, &clientAddr, &clientAddrLen);
        if (bytesReceived < 0) {
            // Handle error
            continue;
        }

        // Process received packet
        if (packet.command == 'H') { // 'H' for Hello
            // create entity for client
            int entity_id = m_ecs->createEntity({FLN_PHYSICS, FLN_TRANSFORM, FLN_TEST, FLN_TESTKILL});
            char* welcome_entity_id = new char[sizeof(entity_id)];
            memcpy(welcome_entity_id, &entity_id, sizeof(entity_id));
            Connection conn;
            conn.last_rec_tick = 0;
            conn.socket = serverSocket;
            conn.entity = entity_id;
            uint32_t clientIP = ((struct sockaddr_in*)&clientAddr)->sin_addr.s_addr;
            addConnection(clientIP, conn);
            Packet welcomePacket;
            welcomePacket.tick = packet.tick; // or set your own tick
            welcomePacket.command = 'W'; // 'W' for Welcome
            welcomePacket.data = welcome_entity_id;

            sendto(serverSocket, (char*)&welcomePacket, sizeof(welcomePacket), 0, &clientAddr, clientAddrLen);

            delete[] welcome_entity_id;
        } else if (packet.command == 'D') { // 'D' for Data
            TickData data;
            Connection client_conn;
            auto it = m_connectionMap.find(((struct sockaddr_in*)&clientAddr)->sin_addr.s_addr);
            if (it == m_connectionMap.end()) {
                // Key not found:
                throw std::runtime_error("Connection not found in the map");
            } else {
                client_conn = it->second;
            }
            // update client's last received tick
            client_conn.last_rec_tick = packet.tick;
            // Populate data based on received Packet
            data.tick = packet.tick;
            size_t dataSize = sizeof(packet.data); // might just have to make this a constant
            char* dataPtr = new char[dataSize]; // Remember to delete this when it gets popped
            memcpy(dataPtr, packet.data, dataSize);
            data.data = dataPtr;
            {
                std::lock_guard<std::mutex> lock(tickBufferMutex);
                client_conn.tickBuffer.push(data);
            }
        }
    }

    close(serverSocket);
}

Gamestate* Network::popLeastRecentGamestate() {
    return nullptr;

}

void Network::addConnection(uint32_t ip, Connection conn) {
    m_connectionMap[ip] = conn;
}

void Network::deserializeAllDataIntoECS(ECS* ecs) {
    // TODO: tick checks, authority check (need to get client id)
    Gamestate* curState = popLeastRecentGamestate();
    while (curState != nullptr) {
        ecs->deserializeIntoData(curState->data, curState->data_size, nullptr);
    }
}

void Network::connect(const char* ip, const char* port) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to create socket\n");
        return;
    }

    // Send Hello packet
    Packet helloPacket;
    helloPacket.tick = 0; // Set your initial tick
    helloPacket.command = 'H'; // 'H' for Hello
    // TODO: set your entity id
    helloPacket.data = nullptr; 

    struct sockaddr_in servAddr;

    sendto(sockfd, (char*)&helloPacket, sizeof(helloPacket), 0, (struct sockaddr *)&servAddr, sizeof(servAddr));

    // Wait for Welcome packet
    Packet welcomePacket;
    recvfrom(sockfd, (char*)&welcomePacket, sizeof(welcomePacket), 0, NULL, NULL);

    if (welcomePacket.command == 'W') {
        Connection conn;
        char* entity_id = welcomePacket.data;
        memcpy(&conn.entity, entity_id, sizeof(conn.entity));
        conn.last_rec_tick = 0;
        conn.socket = sockfd;
        conn.entity = -1; // -1 for server
        uint32_t serverIP = ((struct sockaddr_in*)&servAddr)->sin_addr.s_addr;
        addConnection(serverIP, conn);
        // what do we do with the entity id?
    }

    freeaddrinfo(servinfo); // all done with this structure

    // You might want to save sockfd for future sendto/recvfrom calls.
}

void Network::shutdown() {
    m_shutdown = true;
    m_listenThread.join();
}


int Network::setupUDPConn(const char* address, const char* port) {
    int sock;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_flags = AI_PASSIVE; // use my IP

    std::string defaultPortStr = std::to_string(default_port); // Convert default port to string

    if ((rv = getaddrinfo(address, defaultPortStr.c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind socket\n");
        return -2;
    }

    freeaddrinfo(servinfo); // all done with this structure
    return sock;
}

// TODO:

// 4. Implement Client/Server first tick normalization (maybe just have server send a tick)

// 5. Implement pop_ticks_from_all_connections 

// 6. Add main loop that pops all Tick buffers from conn maps, calls OR_with_authority(?),
//    and calls deserializeAllGameData

// 7. Discuss main loop with team
