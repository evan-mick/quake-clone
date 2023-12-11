#include "network.h"
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include "game_types.h"
#include <iostream>
#include "../game_create_helpers.h"
#include <functional>

/* TODO:
    
    -- INSTANTIATE CLIENT AUTHORITY MAP
    -- USE CLIENT AUTHORITY MAP TO DETERMINE IF CLIENT HAS AUTHORITY OVER ENTITY IN DESERIALIZEALLDATAINTOECS

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

        // Start listening for connections
        // Null addr means listen socket
        int serverSocket = setupUDPConn(nullptr, DEFAULT_PORT);
        if (serverSocket < 0) {
            std::cout << "Unable to set up UDP connection for server" << std::endl;
            return;
        }

        // Open listen thread and accept connections
        m_listenThread = std::thread([this, ip, serverSocket]() {
                this->serverListen(ip, DEFAULT_PORT, serverSocket); 
        });
        m_listenThread.detach();
    }

    // Initialize client
    else if (!server){

        // Attempt to connect to server
        int clientSetup = connect(ip, DEFAULT_PORT); 
        if (clientSetup != 0) {
            throw std::runtime_error("Client initialization failed");
        }

        std::cout << "Connection Established" << std::endl;

        // Populate Client authority vector
        // TODO

        // Open listen thread
        m_listenThread = std::thread([this]() { this->clientListen(); });
        m_listenThread.detach();
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

void Network::serverListen(const char* ip, const char* port, int serverSocket) {

    

    std::cout << "Server now listening on socket: " << std::to_string(serverSocket) << std::endl;

    while (!m_shutdown) {

        struct sockaddr clientAddr {};
        socklen_t clientAddrLen = sizeof(clientAddr);

        char* rec_buff = new char[FULL_PACKET];
        
        // Receive packet
        int bytesReceived = recvfrom(serverSocket, rec_buff, FULL_PACKET, 0, &clientAddr, &clientAddrLen);
        if (bytesReceived < 0) {
            // Handle error
            std::cout << "Packet received error: " << serverSocket << " " << bytesReceived << std::endl;
            perror("server error: ");
            continue;
        }

        std::cout << "Packet received successfully with size: " << bytesReceived << std::endl;

        Packet packet = *((Packet*)rec_buff);

        // Process received packet
        if (packet.command == 'H') { // 'H' for Hello

            // Create entity for client
            entity_t entity_id = createPlayer(m_ecs, glm::vec3(0, 3.f, 0));
            char* welcome_entity_id = new char[sizeof(entity_id)];
            memcpy(welcome_entity_id, &entity_id, sizeof(entity_id));
            
            // Add stuff about client authority 
            m_hasAuthority[entity_id] = false; 

            // Make new Connection
            Connection* conn = new Connection();
            conn->last_rec_tick = packet.tick;

            conn->entity = entity_id;
            conn->ip = (uint32_t)((struct sockaddr_in*)&clientAddr)->sin_addr.s_addr;
            conn->port = (uint16_t)((struct sockaddr_in*)&clientAddr)->sin_port;
            uint32_t clientIP = ((struct sockaddr_in*)&clientAddr)->sin_addr.s_addr;

            // Create send socket to new connection
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(conn->ip), ip, INET_ADDRSTRLEN);
            conn->socket = setupUDPConn(ip, std::to_string(conn->port).c_str());


            // Add to connestions map
            this->addConnection(clientIP, conn);
            std::cout << "connection added" << std::endl;

            // Construct welcome packet
            Packet welcomePacket;
            welcomePacket.tick = m_timer.getTimesRun(); 
            welcomePacket.command = 'W'; // 'W' for Welcome
            // welcomePacket.data = welcome_entity_id;

            char* send_buff = new char[sizeof(Packet) + sizeof(entity_t)];
            memcpy(send_buff, &welcomePacket, sizeof(Packet));
            memcpy(send_buff + sizeof(Packet), &welcome_entity_id, sizeof(entity_t));

            // Send welcome packet
            sendto(serverSocket, send_buff, sizeof(Packet) + sizeof(entity_t), 0, &clientAddr, clientAddrLen);
            std::cout << "welcome packet sent" << std::endl;

            // Clean up
            delete[] welcome_entity_id;
            delete[] send_buff;

        } else if (packet.command == 'D') { // 'D' for Data

            // Get tick at which packet was received
            unsigned int tick = m_timer.getTimesRun();

            // Find connection based on IP
            TickData data;
            
            // Get the connection
            Connection* conn = getConnection(((struct sockaddr_in*)&clientAddr)->sin_addr.s_addr);

            // If not found, throw error
            if (conn == nullptr) {
                std::cout << "Connection not found in the map" << std::endl;
                continue;
            } 

            // Edit the connection to update last received tick
            editConnection(conn->ip, tick);

            // Populate data based on received Packet
            updateTickBuffer(rec_buff + sizeof(Packet), conn, tick);
        }
        delete[] rec_buff;
    }
    close(serverSocket);
    for (auto& [key, val] : m_connectionMap) {
        close(val->socket);
    }
}

void Network::addConnection(uint32_t ip, Connection* conn) {

    // Add connection to map
    // std::lock_guard<std::mutex> lock(m_connectionMutex); // lock the connection map

    m_connectionMutex.lock();
    if (m_connectionMap.find(ip) == m_connectionMap.end()) {
        m_connectionMap[ip] = conn;
        std::cout << "connection added to map" << std::endl;
    } else {
        std::cout << "connection already exists in map" << std::endl;
    }
    m_connectionMutex.unlock();
//    std::cout << "add connection failed, try lock failed" << std::endl;

}

Connection* Network::getConnection(uint32_t ip) {

    // Find connection based on IP
    // std::lock_guard<std::mutex> lock(m_connectionMutex); // Lock the connection map
    if (m_connectionMutex.try_lock()) {
        auto it = m_connectionMap.find(ip);
        if (it == m_connectionMap.end()) {
            // Key not found:
            return nullptr;
        } else {
            return it->second;
        }
    }

    return nullptr;

}

void Network::deserializeAllDataIntoECS(ECS* ecs) {
    
    // Iterate through all connections, pop once per tick
//    m_connectionMutex.lock();
    std::lock_guard<std::mutex> lock(m_connectionMutex);
    for (auto& [ip, conn] : m_connectionMap) {


        if (conn == nullptr) {
            std::cout << "conn is null for ip: " << std::to_string(ip) << std::endl;
            continue;
        }

        TickData* td;
        conn->tick_buffer.mutex.lock();

        if (!conn->tick_buffer.buffer.empty()) {

            std::cout << "conn ip: " << std::to_string(ip) << " " << std::to_string((long)(conn)) << std::endl;
            std::cout << "entity: " << std::to_string((unsigned int)conn->entity) << std::endl; // This might error

            // Get the first element in the tick buffer
            td = conn->tick_buffer.buffer.front();

            char* buff = new char[FULL_PACKET];
            memcpy(buff, td->data, FULL_PACKET);

            // Deserialize data into ECS // NEED TO EDIT THIS TO HANDLE CLIENT AUTHORITY
            ecs->deserializeIntoData(buff, FULL_PACKET, nullptr);

            // Pop the tick buffer
            conn->tick_buffer.buffer.pop();

            // Delete data
            delete[] buff;
        }
        conn->tick_buffer.mutex.unlock();
    }

//    m_connectionMutex.unlock();
}

int Network::connect(const char* ip, const char* port) {

    int sockfd;
    struct addrinfo hints {}, *servinfo, *p;
    int rv;

    std::cout << "Connect start" << std::endl;

    // Set up UDP connection
//    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    // Resolve the address and port
    if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // Loop through all the results and make a socket
    for(p = servinfo; p != nullptr; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        break;
    }

    if (p == nullptr) {
        fprintf(stderr, "client: failed to create socket\n");
        return -1;
    }

//    int server = setupUDPConn(ip, port);

    // Send Hello packet
    Packet helloPacket;
    helloPacket.tick = m_timer.getTimesRun(); 
    helloPacket.command = 'H'; // 'H' for Hello
//    helloPacket.data = nullptr;

    struct sockaddr_in servAddr;
    socklen_t servAddr_len = sizeof(servAddr);
    servAddr.sin_addr.s_addr = ((struct sockaddr_in*)p->ai_addr)->sin_addr.s_addr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = ((struct sockaddr_in*)p->ai_addr)->sin_port;

//    struct sockaddr_in servAddr;
//    socklen_t servAddr_len = sizeof(servAddr);
//    inet_pton(AF_INET, ip, &servAddr.sin_addr);
//    servAddr.sin_family = AF_INET;
//    servAddr.sin_port = std::stoi(port);//((struct sockaddr_in*)p->ai_addr)->sin_port;

    sendto(sockfd, (char*)&helloPacket, sizeof(helloPacket), 0, (struct sockaddr *)&servAddr, servAddr_len);

    // Wait for Welcome packet
//    Packet welcomePacket;
    char* buff = new char[FULL_PACKET];

    // Listen on that port for a welcome packet
    int bytes = recvfrom(sockfd, buff, FULL_PACKET, 0, (struct sockaddr *)&servAddr, &servAddr_len);
    Packet pack = *((Packet*) buff);


    if (pack.command == 'W') {

        std::cout << "Welcome packet received bytes read " << bytes << std::endl;
        // Create new Connection
        Connection* conn = new Connection();
        entity_t entity_id = *((entity_t*)(buff + sizeof(Packet)));
        //assert(entity_id != nullptr);

        //client has authority over this
        m_hasAuthority[entity_id] = true;
        m_myPlayerEntityID = entity_id;

        
        // Create new Connection
        conn->last_rec_tick = pack.tick;
        conn->socket = sockfd;
        conn->entity = MAX_ENT_VAL; // MAX_ENDT_VAL (-1) for server
        conn->ip = ((struct sockaddr_in*)p->ai_addr)->sin_addr.s_addr;
        conn->port = ((struct sockaddr_in*)p->ai_addr)->sin_port;
        //uint32_t serverIP = //((struct sockaddr_in*)p->ai_addr)->sin_addr.s_addr;
        // Add to connestions map
        addConnection(conn->ip, conn);
    }

//    freeaddrinfo(servinfo); // all done with this structure
    return 0;
}

void Network::shutdown() {

    std::lock_guard<std::mutex> lock(m_connectionMutex);
    for (auto& conn : m_connectionMap) {

        // Close all sockets
        close(conn.second->socket);

        // Delete all Tick Data in the buffer
        conn.second->tick_buffer.mutex.lock();
        while (!conn.second->tick_buffer.buffer.empty()) {
            TickData* data = conn.second->tick_buffer.buffer.front();
            conn.second->tick_buffer.buffer.pop();
            delete[] data->data;
            delete data;
        }
        conn.second->tick_buffer.mutex.unlock();

        // Delete the connection
        delete conn.second;
    }

    // Clear the map
    m_connectionMap.clear();

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
                           p->ai_protocol)) < 0) {
            perror("server: socket");
            continue;
        }
        
        if (bind(sock, p->ai_addr, p->ai_addrlen) < 0) {
            close(sock);
            perror("server: bind");
            continue;
        }
        std::cout << "socket bound " << sock << std::endl;
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind socket\n");
        return -2;
    }

//    freeaddrinfo(servinfo);
    return sock;
}


void Network::broadcastGS(ECS* ecs, Connection* conn, int tick) {

    // Instantiate new TickData
    TickData* td = new TickData;
    char* tick_data;

    // Serialize data
    int data_written = ecs->serializeData(&tick_data);
    assert(data_written <= FULL_PACKET - sizeof(Packet));
    td->data = tick_data;
    td->tick = tick;
    
    // Make new packet
    Packet dataPacket;
    dataPacket.tick = td->tick;
    dataPacket.command = 'D'; // 'D' for Data
    char* data = new char[data_written];
    memcpy(data, &dataPacket, sizeof(Packet));
    memcpy(data + sizeof(Packet), td->data, data_written);


    std::cout << std::string(data) << std::endl;
    // Send data to server
    struct sockaddr_in connAddr;
    socklen_t connAddr_len = sizeof(connAddr);
    connAddr.sin_addr.s_addr = conn->ip;
    connAddr.sin_family = AF_INET;
    connAddr.sin_port = conn->port;
    sendto(conn->socket, (char*)&data, sizeof(dataPacket) + data_written, 0, (struct sockaddr *)&connAddr, connAddr_len);

    // Clean up
    delete[] data;
    delete[] td->data;
    delete td;
}

void Network::clientListen() {

    // Get server socket from connection map
    while (!m_shutdown) {

        // Iterate through all connections and find the server (should only be one)
        std::lock_guard<std::mutex> lock(m_connectionMutex);

        for (auto& [ip, conn] : this->m_connectionMap) {

            std::cout << " conn " << ip << " " << (long)(conn) << std::endl;

            // This is the server
            if (conn->entity == MAX_ENT_VAL) {

                int servSocket = conn->socket;

                // Construct packet
//                Packet packet;
                char* buff = new char[FULL_PACKET];
                // memset(buff, 0, FULL_PACKET);

                uint32_t serverIP = conn->ip;
                uint16_t serverPort = conn->port;

                // Receive packet
                struct sockaddr_in servAddr;
                socklen_t servAddr_len = sizeof(servAddr);
                servAddr.sin_addr.s_addr = serverIP;
                servAddr.sin_family = AF_INET;
                servAddr.sin_port = serverPort;

                std::cout << "Attempting to receive on socket" << servSocket << std::endl;
                int bytes = recvfrom(servSocket, buff, FULL_PACKET, 0, (struct sockaddr *)&servAddr, &servAddr_len);
                std::cout << "Received: " << std::to_string(bytes) << std::endl;
                std::cout << "content: " << std::string(buff) << std::endl;


                assert(bytes <= FULL_PACKET);
                assert(bytes > 0);

                // Read into packet
                Packet packet;
                memcpy(&packet, buff, sizeof(Packet));

                // Separate data from packet
                char* data = new char[bytes - sizeof(Packet)];
                memcpy(data, buff + sizeof(Packet), bytes - sizeof(Packet));

                // Process packet
                if (packet.command == 'D') {
                    std::cout << "D Command" << std::endl;
                    // Populate data based on received Packet
                    unsigned int tick = m_timer.getTimesRun();
                    updateTickBuffer(data, conn, tick);
                    
                } else {
                    std::cout << "Unknown command: " << std::to_string(packet.command) << std::endl;
                }
                delete[] buff;
                delete[] data;
                continue;
            }
        }
    }
}

void Network::updateTickBuffer(char* data, Connection* conn, unsigned int tick) {

    // Populate data based on received Packet
    TickData* td = new TickData;
    
    // Copy data into buffer
    char* data_buff = new char[FULL_PACKET - sizeof(Packet)];
    memcpy(data_buff, data, FULL_PACKET - sizeof(Packet));

    // Populate TickData
    td->tick = tick;
    td->data = data_buff;

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

void Network::pushTickData(TickData* td, Connection* conn) {

    std::cout << "locking tick data mutex" << std::endl;
    // Lock the tick buffer
    conn->tick_buffer.mutex.lock();

    std::cout << "pushing tick data" << std::endl;
    std::cout << "buffer size: " << std::to_string(conn->tick_buffer.buffer.size()) << std::endl;
    // Push data into tick buffer
    conn->tick_buffer.buffer.push(td);
    std::cout << "pushed tick data" << std::endl;
    std::cout << "buffer size: " << std::to_string(conn->tick_buffer.buffer.size()) << std::endl;

    // Unlock the tick buffer
    conn->tick_buffer.mutex.unlock();
    std::cout << "tick data mutex unlocked" << std::endl;
    
}

void Network::onTick(unsigned int tick) {

    // Deserialize all data into ECS to update the gamestate
    deserializeAllDataIntoECS(m_ecs);
    
    // Broadcast gamestate to all connections
    std::lock_guard<std::mutex> lock(m_connectionMutex); 
    for (auto& conn : m_connectionMap) {
        
        // Assert GS is getting sent to the right place
        if (!m_isServer) assert(conn.second->entity == MAX_ENT_VAL);
        if (m_isServer) assert(conn.second->entity != MAX_ENT_VAL);

        // Send gamestate to connection
        broadcastGS(m_ecs, conn.second, tick);
    }
}


