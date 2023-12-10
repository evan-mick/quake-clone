#include "network.h"
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include "../game_types.h"

/* TODO:
    
    -- Deal with client authority

*/

const char* DEFAULT_PORT = "42069";

Network::Network(bool server, ECS* ecs, const char* ip)
{
    // IP and port are ALWAYS the server address and port

    m_isServer = server;
    m_ecs = ecs;
    m_shutdown = false;

    // Initialize Tick Rate
    m_timer.setAndResetTimer(TICK_RATE); // 60 ticks per second
    
    // Initialize server
    if (server) {


        // Server has authority over everything by default
        std::fill(m_hasAuthority.begin(), m_hasAuthority.end(), true);

        // Open listen thread and accept connections
        m_listenThread = std::thread([this, ip]() { 
            this->serverListen(ip, DEFAULT_PORT); 
        });
    }

    // Initialize client
    if (!server){

        // Attempt to connect to server
        int clientSetup = connect(ip, DEFAULT_PORT); 
        if (clientSetup != 0) {
            throw std::runtime_error("Client initialization failed");
        }

        // Populate Client authority vector
        // TODO

        // Open listen thread
        m_listenThread = std::thread([this]() { this->clientListen(); });
    }

}

void Network::mainLoop(float delta) {

    if (!m_shutdown) {
        
        m_timer.increment(delta);

        // Wait for tick
        if (!m_timer.finishedThenResetTime()) { 
            return;
        }

        // Get tick
        unsigned int tick = m_timer.getTimesRun();

        // Pop tick buffers and deserialize into ECS
        onTick(tick);
    }

    
}

void Network::serverListen(const char* ip, const char* port) {

    int serverSocket = setupUDPConn(ip, port); 
    if (serverSocket < 0) {
        throw std::runtime_error("Unable to set up UDP connection for server");
    }

    while (!m_shutdown) {
        Packet packet;
        struct sockaddr clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        // Receive packet
        int bytesReceived = recvfrom(serverSocket, (char*)&packet, sizeof(packet), 0, &clientAddr, &clientAddrLen);
        if (bytesReceived < 0) {
            // Handle error
            continue;
        }

        // Process received packet
        if (packet.command == 'H') { // 'H' for Hello
            // Create entity for client
            int entity_id = m_ecs->createEntity({FLN_PHYSICS, FLN_TRANSFORM, FLN_TEST, FLN_TESTKILL});
            char* welcome_entity_id = new char[sizeof(entity_id)];
            memcpy(welcome_entity_id, &entity_id, sizeof(entity_id));
            
            // Add stuff about client authority 
            m_hasAuthority[entity_id] = false; 

            // Make new Connection
            Connection conn;
            conn.last_rec_tick = packet.tick;
            conn.socket = serverSocket;
            conn.entity = entity_id;
            conn.ip = (uint32_t)((struct sockaddr_in*)&clientAddr)->sin_addr.s_addr;
            conn.port = (uint16_t)((struct sockaddr_in*)&clientAddr)->sin_port;
            uint32_t clientIP = ((struct sockaddr_in*)&clientAddr)->sin_addr.s_addr;

            // Add to connestions map
            addConnection(clientIP, &conn);

            // Construct welcome packet
            Packet welcomePacket;
            welcomePacket.tick = m_timer.getTimesRun(); 
            welcomePacket.command = 'W'; // 'W' for Welcome
            welcomePacket.data = welcome_entity_id;

            // Send welcome packet
            sendto(serverSocket, (char*)&welcomePacket, sizeof(welcomePacket), 0, &clientAddr, clientAddrLen);

            // Clean up
            delete[] welcome_entity_id;

        } else if (packet.command == 'D') { // 'D' for Data
            // Get tick at which packet was received
            unsigned int tick = m_timer.getTimesRun();

            // Find connection based on IP
            TickData data;
            
            // Get the connection
            Connection* conn = getConnection(((struct sockaddr_in*)&clientAddr)->sin_addr.s_addr);

            // If not found, throw error
            if (conn == nullptr) {
                throw std::runtime_error("Connection not found in the map");
            } 

            // Edit the connection to update last received tick
            editConnection(conn->ip, tick);

            // Populate data based on received Packet
            updateTickBuffer(packet, conn, tick);
        }
    }
    close(serverSocket);
}

void Network::addConnection(uint32_t ip, Connection* conn) {

    // Add connection to map
    std::lock_guard<std::mutex> lock(m_connectionMutex); // lock the connection map
    m_connectionMap[ip] = conn;
}

Connection* Network::getConnection(uint32_t ip) {

    // Find connection based on IP
    std::lock_guard<std::mutex> lock(m_connectionMutex); // Lock the connection map
    auto it = m_connectionMap.find(ip);
    if (it == m_connectionMap.end()) {
        // Key not found:
        return nullptr;
    } else {
        return it->second;
    }

}

void Network::deserializeAllDataIntoECS(ECS* ecs) {
    
    // Iterate through all connections, pop once per tick
    m_connectionMutex.lock();
    for (auto& conn : m_connectionMap) { 

        TickData data;
        conn.second->tick_buffer.mutex.lock();

        if (!conn.second->tick_buffer.buffer.empty()) {

            // Pop data
            data = conn.second->tick_buffer.buffer.front();
            conn.second->tick_buffer.buffer.pop();

            // Deserialize data into ECS
            ecs->deserializeIntoData(data.data, sizeof(data.data), nullptr);

            // Delete data
            delete[] data.data;
        }
        conn.second->tick_buffer.mutex.unlock();
    }

    m_connectionMutex.unlock();
}

int Network::connect(const char* ip, const char* port) {

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    // Set up UDP connection
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    // Resolve the address and port
    if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // Loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to create socket\n");
        return -1;
    }

    // Send Hello packet
    Packet helloPacket;
    helloPacket.tick = m_timer.getTimesRun(); 
    helloPacket.command = 'H'; // 'H' for Hello
    helloPacket.data = nullptr; 

    struct sockaddr_in servAddr;
    socklen_t servAddr_len = sizeof(servAddr);
    servAddr.sin_addr.s_addr = ((struct sockaddr_in*)p->ai_addr)->sin_addr.s_addr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = ((struct sockaddr_in*)p->ai_addr)->sin_port;

    sendto(sockfd, (char*)&helloPacket, sizeof(helloPacket), 0, (struct sockaddr *)&servAddr, servAddr_len);

    // Wait for Welcome packet
    Packet welcomePacket;

    // Listen on that port for a welcome packet
    recvfrom(sockfd, (char*)&welcomePacket, sizeof(welcomePacket), 0, (struct sockaddr *)&servAddr, &servAddr_len);

    if (welcomePacket.command == 'W') {
        // Create new Connection
        Connection conn;
        char* entity_id = welcomePacket.data;
        assert(entity_id != nullptr);

        //client has authority over this
        m_hasAuthority[*entity_id] = true;
        m_myPlayerEntityID = *entity_id;
        
        // Create new Connection
        conn.last_rec_tick = welcomePacket.tick;
        conn.socket = sockfd;
        conn.entity = -1; // -1 for server
        conn.ip = (uint32_t)((struct sockaddr_in*)p->ai_addr)->sin_addr.s_addr;
        conn.port = (uint16_t)((struct sockaddr_in*)p->ai_addr)->sin_port;
        uint32_t serverIP = ((struct sockaddr_in*)p->ai_addr)->sin_addr.s_addr;
        // Add to connestions map
        addConnection(serverIP, &conn);
    }

    freeaddrinfo(servinfo); // all done with this structure
    return 0;
}

void Network::shutdown() {

    m_connectionMutex.lock();
    for (auto& conn : m_connectionMap) {

        // Close all sockets
        close(conn.second->socket);

        // Delete all Tick Data in the buffer
        conn.second->tick_buffer.mutex.lock();
        while (!conn.second->tick_buffer.buffer.empty()) {
            TickData data = conn.second->tick_buffer.buffer.front();
            conn.second->tick_buffer.buffer.pop();
            delete[] data.data;
        }
        conn.second->tick_buffer.mutex.unlock();

        // Delete the connection
        delete conn.second;
    }

    // Clear the map
    m_connectionMap.clear();
    m_connectionMutex.unlock();

    // Set shutdown flag
    m_shutdown = true;
    m_listenThread.join();
}


int Network::setupUDPConn(const char* address, const char* port) {

    int sock;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_flags = AI_PASSIVE; // use my IP

    // Resolve the address and port
    if ((rv = getaddrinfo(address, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // Create a listening socket for the server on default port
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
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

    freeaddrinfo(servinfo); 
    return 0;
}


void Network::broadcastGS(ECS* ecs, Connection* conn, int tick) {

    // Instantiate new TickData
    TickData* td = new TickData;
    char* tick_data = new char[1400];

    // Serialize data
    int data_written = ecs->serializeData(&tick_data);
    assert(data_written <= 1400);
    td->data = tick_data;
    td->tick = tick;
    
    // Make new packet
    Packet dataPacket;
    dataPacket.tick = td->tick;
    dataPacket.command = 'D'; // 'D' for Data
    char* data = new char[data_written];
    memcpy(data, td->data, data_written);
    dataPacket.data = data;

    // Send data to server
    struct sockaddr_in connAddr;
    socklen_t connAddr_len = sizeof(connAddr);
    connAddr.sin_addr.s_addr = conn->ip;
    connAddr.sin_family = AF_INET;
    connAddr.sin_port = conn->port;
    sendto(conn->socket, (char*)&dataPacket, sizeof(dataPacket), 0, (struct sockaddr *)&connAddr, connAddr_len);

    // Clean up
    delete[] data;
    delete[] td->data;
    delete td;
}

void Network::clientListen() {

    // Get server socket from connection map
    while (!m_shutdown) {

        // Initialize server socket to something invalid
        int servSocket = -1;

        // Iterate through all connections and find the server (should only be one)
        m_connectionMutex.lock();
        for (auto& conn : m_connectionMap) {

            // This is the server
            if (conn.second->entity == -1) {

                servSocket = conn.second->socket;

                // Construct packet
                Packet packet;
                uint32_t serverIP = conn.second->ip;
                uint16_t serverPort = conn.second->port;

                // Receive packet
                struct sockaddr_in servAddr;
                socklen_t servAddr_len = sizeof(servAddr);
                servAddr.sin_addr.s_addr = serverIP;
                servAddr.sin_family = AF_INET;
                servAddr.sin_port = serverPort;
                recvfrom(servSocket, (char*)&packet, sizeof(packet), 0, (struct sockaddr *)&servAddr, &servAddr_len);

                // Process packet
                if (packet.command == 'D') {
                    // Populate data based on received Packet
                    unsigned int tick = m_timer.getTimesRun();
                    updateTickBuffer(packet, conn.second, tick);
                }
                break;
            }
        }

        m_connectionMutex.unlock();
    }
}

void Network::updateTickBuffer(Packet packet, Connection* conn, unsigned int tick) {

    // Populate data based on received Packet
    TickData td;
    td.tick = tick;
    size_t dataSize = sizeof(packet.data); 
    char* dataPtr = new char[dataSize]; 
    memcpy(dataPtr, packet.data, dataSize);
    td.data = dataPtr;

    // Push data into tick buffer
    pushTickData(td, conn);
}

void Network::editConnection(uint32_t ip, unsigned int tick) {

    // Find connection based on IP
    Connection* conn = getConnection(ip);

    std::lock_guard<std::mutex> lock(m_connectionMutex); // lock the connection map after getting the connection
    if (conn == nullptr) {
        throw std::runtime_error("Connection not found in the map");
    }
    conn->last_rec_tick = tick;
}

void Network::pushTickData(TickData td, Connection* conn) {

    // Lock the tick buffer
    conn->tick_buffer.mutex.lock(); 

    // Push data into tick buffer
    conn->tick_buffer.buffer.push(td);

    // Unlock the tick buffer
    conn->tick_buffer.mutex.unlock();
}

void Network::onTick(unsigned int tick) {

    // Deserialize all data into ECS to update the gamestate
    deserializeAllDataIntoECS(m_ecs);
    
    // Broadcast gamestate to all connections
    std::lock_guard<std::mutex> lock(m_connectionMutex); 
    for (auto& conn : m_connectionMap) {
        
        // Assert GS is getting sent to the right place
        if (!m_isServer) assert(conn.second->entity == -1);
        if (m_isServer) assert(conn.second->entity != -1);

        // Send gamestate to connection
        broadcastGS(m_ecs, conn.second, tick);
    }
}


