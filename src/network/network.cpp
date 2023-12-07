#include "network.h"
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include "../game_types.h"

// TODO //

// Implement onTick(), and init() functions and syncromize with Network main loop
// Find a way to get server tick rate and client tick rate to match, adjust broadcastClientGS() and listenThread() accordingly


Network::Network(bool server, ECS* ecs)
{
    m_isServer = server;
    m_ecs = ecs;

    // Server has authority over everything by default
    if (server) {
        std::fill(m_hasAuthority.begin(), m_hasAuthority.end(), true);
        // initialize server and open listen thread
        m_listenThread = std::thread([this]() { this->serverListen(); });
    }

    // Initialize client
    
    if (!server){
        int initSuccess = initClient();
        if (initSuccess != 0) {
            throw std::runtime_error("Network initialization failed");
        }
        m_listenThread = std::thread([this]() { this->clientListen(); });
    }

    
    // Need ticks to be implemented

    // while (!m_shutdown) {
    //     onTick();
    // }

}


void Network::serverListen() {
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof(their_addr);

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
            // Make new Connection
            Connection conn;
            conn.last_rec_tick = 0;
            conn.socket = serverSocket;
            conn.entity = entity_id;
            conn.ip = (uint32_t)((struct sockaddr_in*)&clientAddr)->sin_addr.s_addr;
            conn.port = (uint16_t)((struct sockaddr_in*)&clientAddr)->sin_port;
            uint32_t clientIP = ((struct sockaddr_in*)&clientAddr)->sin_addr.s_addr;
            addConnection(clientIP, &conn);
            Packet welcomePacket;
            welcomePacket.tick = packet.tick; // or set your own tick
            welcomePacket.command = 'W'; // 'W' for Welcome
            welcomePacket.data = welcome_entity_id;

            sendto(serverSocket, (char*)&welcomePacket, sizeof(welcomePacket), 0, &clientAddr, clientAddrLen);

            delete[] welcome_entity_id;
        } else if (packet.command == 'D') { // 'D' for Data
            TickData data;
            Connection* client_conn;
            auto it = m_connectionMap.find(((struct sockaddr_in*)&clientAddr)->sin_addr.s_addr);
            if (it == m_connectionMap.end()) {
                // Key not found:
                throw std::runtime_error("Connection not found in the map");
            } else {
                client_conn = it->second;
            }
            // CHANGE THIS
            int tick = packet.tick;
            // Populate data based on received Packet
            updateTickBuffer(packet, client_conn, tick);
        }
    }

    close(serverSocket);
}

void Network::addConnection(uint32_t ip, Connection* conn) {
    std::lock_guard<std::mutex> lock(m_connectionMutex); // lock the connection map
    m_connectionMap[ip] = conn;
}

Connection* Network::getConnection(uint32_t ip) {
    std::lock_guard<std::mutex> lock(m_connectionMutex); // lock the connection map
    auto it = m_connectionMap.find(ip);
    if (it == m_connectionMap.end()) {
        // Key not found:
        return nullptr;
    } else {
        return it->second;
    }
}
void Network::deserializeAllDataIntoECS(ECS* ecs) {
    // TODO: tick checks, authority check (need to get client id)
    m_connectionMutex.lock();
    for (auto& conn : m_connectionMap) { // iterate through all connections
                                        // pop once per tick
        TickData data;
        conn.second->tick_buffer.mutex.lock();
        if (!conn.second->tick_buffer.buffer.empty()) {
            data = conn.second->tick_buffer.buffer.front();
            // if server, broadcast to all clients
            conn.second->tick_buffer.buffer.pop();
            ecs->deserializeIntoData(data.data, sizeof(data.data), nullptr);
            delete[] data.data;
            // Need a way to broadcast new gamestate to all clients

        }
        conn.second->tick_buffer.mutex.unlock();
    }
    if (m_isServer) {
        TickData* td = new TickData;
        char* tick_data = new char[1400];
        int data_written = ecs->serializeData(&tick_data);
        assert(data_written <= 1400);
        td->data = tick_data;
        // ADD A WAY TO GET THE SERVER TICK
        td->tick = 0; // ecs->getTick();
        Packet dataPacket;
        dataPacket.tick = td->tick;
        dataPacket.command = 'D'; // 'D' for Data
        char* data = new char[data_written];
        memcpy(data, td->data, data_written);
        dataPacket.data = data;
        for (auto& conn : m_connectionMap) {
            // Send data to all clients
            struct sockaddr_in clientAddr;
            clientAddr.sin_addr.s_addr = conn.second->ip;
            clientAddr.sin_family = AF_INET;
            clientAddr.sin_port = conn.second->port;
            sendto(conn.second->socket, (char*)&dataPacket, sizeof(dataPacket), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
            
        }
        delete[] data;
        delete[] td->data;
        delete td;
    }
    m_connectionMutex.unlock();
}

int Network::connect(const char* ip, const char* port) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
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
        return -1;
    }

    // Send Hello packet
    Packet helloPacket;
    helloPacket.tick = 0; // Set your initial tick
    helloPacket.command = 'H'; // 'H' for Hello
    // TODO: set your entity id
    helloPacket.data = nullptr; 

    struct sockaddr_in servAddr;
    servAddr.sin_addr.s_addr = ((struct sockaddr_in*)p->ai_addr)->sin_addr.s_addr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = ((struct sockaddr_in*)p->ai_addr)->sin_port;

    sendto(sockfd, (char*)&helloPacket, sizeof(helloPacket), 0, (struct sockaddr *)&servAddr, sizeof(servAddr));

    // Wait for Welcome packet
    Packet welcomePacket;

    // Listen on that port for a welcome packet
    recvfrom(sockfd, (char*)&welcomePacket, sizeof(welcomePacket), 0, (struct sockaddr *)&servAddr, NULL);

    if (welcomePacket.command == 'W') {
        // Create new Connection
        Connection conn;
        char* entity_id = welcomePacket.data;
        memcpy(&conn.entity, entity_id, sizeof(conn.entity));
        // Create new Connection
        conn.last_rec_tick = 0;
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

    memset(&hints, 0, sizeof(hints));
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
    return 0;
}


void Network::broadcastClientGS(ECS* ecs, Connection* conn, int tick) {
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
    struct sockaddr_in servAddr;
    servAddr.sin_addr.s_addr = conn->ip;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = conn->port;
    sendto(conn->socket, (char*)&dataPacket, sizeof(dataPacket), 0, (struct sockaddr *)&servAddr, sizeof(servAddr));

    // clean up
    delete[] data;
    delete[] td->data;
    delete td;
}

void Network::clientListen() {
    // Get server socket from connection map
    while (!m_shutdown) {
        int servSocket = -1;
        for (auto& conn : m_connectionMap) {
            if (conn.second->entity == -1) {
                // This is the server
                servSocket = conn.second->socket;
                // recv data from server
                Packet packet;
                uint32_t serverIP = conn.second->ip;
                uint16_t serverPort = conn.second->port;
                struct sockaddr_in servAddr;
                servAddr.sin_addr.s_addr = serverIP;
                servAddr.sin_family = AF_INET;
                servAddr.sin_port = serverPort;
                recvfrom(servSocket, (char*)&packet, sizeof(packet), 0, (struct sockaddr *)&servAddr, NULL);
                if (packet.command == 'D') {
                    // Populate data based on received Packet
                    int newtick = packet.tick; // CHANGE
                    updateTickBuffer(packet, conn.second, newtick);
                }
                continue;
            }
        }
    }
}

void Network::updateTickBuffer(Packet packet, Connection* conn, int newtick) {
    // Populate data based on received Packet
    TickData data;
    data.tick = newtick;
    size_t dataSize = sizeof(packet.data); // might just have to make this a constant
    char* dataPtr = new char[dataSize]; // Remember to delete this when it gets popped
    memcpy(dataPtr, packet.data, dataSize);
    data.data = dataPtr;
    {
        conn->tick_buffer.mutex.lock();
        conn->tick_buffer.buffer.push(data);
        conn->tick_buffer.mutex.unlock();
    }
}

void Network::onTick() {
    int tick = 0; // CHANGE THIS

    // Night change the order of these two, not sure yet

    // Deserialize all data into ECS to update the gamestate
    deserializeAllDataIntoECS(m_ecs);
    
    // If client, send gamestate to server

    if (!m_isServer) {
        std::lock_guard<std::mutex> lock(m_connectionMutex); // This probably isnt necessary
        for (auto& conn : m_connectionMap) {
            if (conn.second->entity == -1) {
                broadcastClientGS(m_ecs, conn.second, tick); // CHANGE THIS
                break;
            }   
        }
    }
}

int Network::initClient() {  
    
    const char* port = "42069";
    const char* ip = NULL; // CHANGE THIS TO A USER INPUT

    int clientSetup = connect(ip, port); 
    if (clientSetup != 0) {
        // Handle error: unable to set up UDP connection
        return -1;
    }

    return 0;

}

