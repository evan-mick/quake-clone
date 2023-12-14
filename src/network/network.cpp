#include "network.h"
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include "game_types.h"
#include <iostream>
#include "../game_create_helpers.h"
#include <functional>

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

        m_myPlayerEntityID = MAX_ENT_VAL;

        // Open listen thread and accept connections
        m_listenThread = std::thread([this, ip]() {
                this->serverListen(ip, DEFAULT_PORT);
        });
        m_listenThread.detach();
    }

    // Initialize client
    else if (!server){

        // Attempt to connect to server
        int clientSetup = connect(ip, DEFAULT_PORT); 
        if (clientSetup != 0) {
            std::cout << "CLIENT INTIALIZATION FAILED" << std::endl;
        }

        std::cout << "Connection Established" << std::endl;

        // Populate Client authority vector
        next.tick = -1;
        next.data = nullptr;
        next.data_size = 0;
        next.from_conn = nullptr;

        // Open listen thread
        m_listenThread = std::thread([this]() { this->clientListen(); });
        m_listenThread.detach();
    }

}

void Network::serverListen(const char* ip, const char* port) {

    addrinfo* p;
    int serverSocket = setupUDPConn(nullptr, DEFAULT_PORT, true, &p);

    std::cout << "Server now listening on socket: " << std::to_string(serverSocket) << std::endl;

    while (!m_shutdown) {

        struct sockaddr clientAddr {};
        socklen_t clientAddrLen = sizeof(clientAddr);

        char rec_buff[FULL_PACKET];

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

        uint32_t c_ip = ((struct sockaddr_in*)&clientAddr)->sin_addr.s_addr;
        in_port_t c_port = ((struct sockaddr_in*)&clientAddr)->sin_port;

        // Process received packet
        if (packet.command == 'H') { // 'H' for Hello

            // Create entity for client
            entity_t entity_id = createPlayer(m_ecs, glm::vec3(0, 50.f, 0));
            std::cout << "Client entity created -- entityID: " << std::to_string(entity_id) <<std::endl;

            // Add stuff about client authority
            m_ecs->setAuthority(entity_id, false);

            // Make new Connection
            Connection* conn = new Connection();

            conn->last_rec_tick = -1;//packet.tick;
            conn->entity = entity_id;
            conn->ip = c_ip;
            conn->port = c_port;
            conn->lastActivityTime = std::chrono::steady_clock::now();
            conn->timeoutDuration = std::chrono::seconds(5);
            conn->discon_timer = 5.f;


            // Create send socket to new connection
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(conn->ip), ip, INET_ADDRSTRLEN);

            char ipString[INET_ADDRSTRLEN];
            const char* c_ip_str = inet_ntop(AF_INET, &c_ip, ipString, INET_ADDRSTRLEN);
            addrinfo* p;
            conn->socket = serverSocket;

            // Add to connestions map
            this->addConnection(ipport(c_ip, c_port), conn);
            std::cout << "connection added" << std::endl;

            // Construct welcome packet
            Packet welcomePacket;
            welcomePacket.tick = m_timer.getTimesRun();
            welcomePacket.command = 'W'; // 'W' for Welcome

            char send_buff[sizeof(Packet) + sizeof(entity_t)];
            memcpy(send_buff, &welcomePacket, sizeof(Packet));
            memcpy(send_buff + sizeof(Packet), &entity_id, sizeof(entity_t));

            // Send welcome packet
            sendto(serverSocket, send_buff, sizeof(Packet) + sizeof(entity_t), 0, &clientAddr, clientAddrLen);
            std::cout << "welcome packet sent " << (int)(entity_id) << std::endl;

        } else if (packet.command == 'D') { // 'D' for Data
//            std::cout << "Command D from client" << std::endl;

            // Get tick at which packet was received
            unsigned int tick = m_timer.getTimesRun();

            // Get the connection
            Connection* conn = getConnection(ipport(c_ip, c_port));
            conn->discon_timer = 5.f;

            // If not found, throw error
            if (conn == nullptr) {
                std::cout << "Connection not found in the map" << std::endl;
                continue;
            }

            conn->resetTimer();

            // Edit the connection to update last received tick
            editConnection(conn, tick);

            // Populate data based on received Packet
            char* send = new char[bytesReceived - sizeof(Packet)];
            memcpy(send, rec_buff + sizeof(Packet), bytesReceived - sizeof(Packet));

            TickData push;
            push.data = send;
            push.tick = tick;
            push.data_size = bytesReceived - sizeof(Packet);
            push.from_conn = conn;
            buffer.push(push);
        }
    }

    close(serverSocket);
    for (auto& [key, val] : m_connectionMap) {
        close(val->socket);
    }
}


void Network::broadcastOnTick(float delta) {

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


void Network::addConnection(uint64_t ipport_, Connection* conn) {

    // Add connection to map

    m_connectionMutex.lock();
    if (m_connectionMap.find(ipport_) == m_connectionMap.end()) {
        m_connectionMap[ipport_] = conn;
        std::cout << "connection added to map" << std::endl;
    } else {
        std::cout << "connection already exists in map" << std::endl;
    }
    m_connectionMutex.unlock();

}

Connection* Network::getConnection(uint64_t ipport) {

    // Find connection based on IP

        auto it = m_connectionMap.find(ipport);
        if (it == m_connectionMap.end()) {
            // Key not found:
            return nullptr;
        } else {
            return it->second;

        }
    return nullptr;

}

void Network::editConnection(Connection* conn, unsigned int tick) {

    // Update last received tick of connection
    std::lock_guard<std::mutex> lock(m_connectionMutex);
    if (conn == nullptr) {
            std::cout << "Connection not found in the map" << std::endl;
    }
    conn->last_rec_tick = tick;

}

void Network::deserializeAllDataIntoECS() {
    
    // Iterate through all connections, pop once per tick
    std::lock_guard<std::mutex> lock(m_connectionMutex);

    while (!buffer.empty()) {

        TickData& nxt = buffer.front();
        if (m_connectionMap.size() > 0 && nxt.data == nullptr) {
            return;
        }

        char buff[FULL_PACKET];

        // Ignore authority for first established tick
        if (next.tick < 0) {
            m_ecs->deserializeIntoData(nxt.data, nxt.data_size, true);

        }
        else if (nxt.tick >= nxt.from_conn->last_rec_tick) {
            m_ecs->deserializeIntoData(nxt.data, nxt.data_size, false);
            nxt.from_conn->last_rec_tick = nxt.tick;
        }

        delete[] nxt.data;
        nxt.data = nullptr;
        buffer.pop();
    }

    if (next.tick < 0)
        next.tick = 0;
}

int Network::connect(const char* ip, const char* port) {

    std::cout << "Connect start" << std::endl;

    struct addrinfo* p;
    int sockfd = setupUDPConn(ip, port, false, &p);

    // Send Hello packet
    Packet helloPacket;
    helloPacket.tick = m_timer.getTimesRun(); 
    helloPacket.command = 'H'; // 'H' for Hello

    struct sockaddr_in servAddr;
    socklen_t servAddr_len = sizeof(servAddr);
    servAddr = *((struct sockaddr_in*)(p->ai_addr));

    sendto(sockfd, &helloPacket, sizeof(helloPacket), 0, (struct sockaddr *)(&servAddr), servAddr_len);

    // Wait for Welcome packet
    char buff[FULL_PACKET];
    memset(buff, 0, FULL_PACKET);

    // Listen on that port for a welcome packet
    int bytes = recv(sockfd, buff, FULL_PACKET, 0);
    Packet pack;
    memcpy(&pack, buff, sizeof(Packet));

    if (pack.command == 'W') {

        // Create new Connection
        Connection* conn = new Connection();
        entity_t entity_id;

        memcpy(&entity_id, buff + sizeof(Packet), sizeof(entity_t));
        std::cout << "received entity id: " << std::to_string(entity_id) << std::endl;

        // Client has authority over this
        m_ecs->setAuthority(entity_id, true);
        m_myPlayerEntityID = entity_id;

        std::cout << "Welcome packet received bytes read " << bytes << " " << std::to_string((int)entity_id) << std::endl;

        // Create new Connection
        conn->last_rec_tick = pack.tick;
        conn->socket = sockfd;
        conn->entity = MAX_ENT_VAL; // MAX_ENDT_VAL (-1) for server
        conn->ip = servAddr.sin_addr.s_addr;
        conn->port = servAddr.sin_port;
        conn->lastActivityTime = std::chrono::steady_clock::now();
        conn->timeoutDuration = std::chrono::seconds(5);
        conn->discon_timer = 5.f;

        // Add to connestions map
        addConnection(ipport(conn->ip, conn->port), conn);
    }

    freeaddrinfo(p);
    return 0;
}

void Network::shutdown() {

    std::lock_guard<std::mutex> lock(m_connectionMutex);
    for (auto& [ipport, conn] : m_connectionMap) {

        // Close all sockets
        close(conn->socket);

        // Delete all Tick Data in the buffer
        mutex_.lock();
        while (!buffer.empty()) {
            TickData& data = buffer.front();
            buffer.pop();
            delete[] data.data;
        }
        mutex_.unlock();

        // Delete the connection
        delete conn;
    }

    // Clear the map
    m_connectionMap.clear();

    // Set shutdown flag
    m_shutdown = true;
    m_listenThread.join();
}

void Network::broadcastGS(ECS* ecs, Connection* conn, int tick) {

    // Send data to server
    struct sockaddr_in connAddr {};
    socklen_t connAddr_len = sizeof(connAddr);
    connAddr.sin_addr.s_addr = conn->ip;
    connAddr.sin_family = AF_INET;
    connAddr.sin_port = conn->port;


    // Serialize data
    char* tick_data;
    int data_written = 0;
    int total_data_written = 0;
    data_written = ecs->serializeData(&tick_data, m_isServer, FULL_PACKET, total_data_written);
    while (data_written > 0) {

        total_data_written += data_written;

        // Make new packet
        Packet dataPacket {};
        dataPacket.tick = tick;
        dataPacket.command = 'D'; // 'D' for Data
        char* data = new char[data_written + sizeof(Packet)];
        memcpy(data, &dataPacket, sizeof(Packet));
        memcpy(data + sizeof(Packet), tick_data, data_written);

        ssize_t sent = sendto(conn->socket, data, sizeof(dataPacket) + data_written, 0, (struct sockaddr *)&connAddr, connAddr_len);

        // Clean up
        delete[] tick_data;
        delete[] data;
        data_written = ecs->serializeData(&tick_data, m_isServer, FULL_PACKET, total_data_written);

    }
    delete[] tick_data;
}

void Network::clientListen() {

    // Get server socket from connection map
    while (!m_shutdown) {

        // Iterate through all connections and find the server (should only be one).
        for (auto& [ipport, conn] : this->m_connectionMap) {

            conn->discon_timer = 5.f;

            if (conn == nullptr) {
                std::cout << "conn is null for client, null conn ip is " << std::to_string(ipport) << std::endl;
                continue;
            }
            // This is the server
            if (conn->entity == MAX_ENT_VAL) {

                int servSocket = conn->socket;

                // Construct packet
                char buff[FULL_PACKET];
                memset(buff, 0, FULL_PACKET);

                uint32_t serverIP = conn->ip;
                uint16_t serverPort = conn->port;

                // Receive packet
                struct sockaddr_in servAddr;
                socklen_t servAddr_len = sizeof(servAddr);
                servAddr.sin_addr.s_addr = serverIP;
                servAddr.sin_family = AF_INET;
                servAddr.sin_port = serverPort;

                int bytes = recvfrom(servSocket, buff, FULL_PACKET, 0, (struct sockaddr *)&servAddr, &servAddr_len);

                if (bytes == 0) {
                    continue;
                }

                // Read into packet
                Packet packet;
                memcpy(&packet, buff, sizeof(Packet));
                conn->resetTimer();

                // Process packet
                if (packet.command == 'D') {

                    // Separate data from packet
                    char* data = new char[bytes - sizeof(Packet)];
                    memcpy(data, buff + sizeof(Packet), bytes - sizeof(Packet));

                    // Populate data based on received Packet
                    unsigned int tick = m_timer.getTimesRun();

                    if (next.tick >= 0)
                        next.tick = tick;

                    TickData push;
                    push.data = data;
                    push.tick = tick;
                    push.from_conn = conn;
                    if (next.tick >= 0)
                        next.tick = tick;

                    push.data_size = bytes - sizeof(Packet);
                    buffer.push(push);
                    
                } else {
                    std::cout << "Unknown command: " << std::to_string(packet.command) << std::endl;
                }
                continue;
            }
        }
    }
}

void Network::updateTickBuffer(char* data, size_t data_size, Connection* conn, unsigned int tick) {

    // Populate TickData based on received Packet
    TickData td {};
    td.tick = tick;
    td.data = data;
    td.data_size = data_size;
    td.from_conn = conn;

    // Push data into tick buffer
    std::lock_guard<std::mutex> lock(mutex_);
    buffer.push(td);
}

void Network::onTick(unsigned int tick) {
    // Broadcast gamestate to all connections
    std::lock_guard<std::mutex> lock(m_connectionMutex);
    for (auto& [ip, conn] : m_connectionMap) {
        if (conn == nullptr) {
            std::cout << "null connection in broadcast" << std::endl;
            continue;
        }

        if (!conn->hasTimedOut()) {
            // Send gamestate to connection
            broadcastGS(m_ecs, conn, tick);
        } else {
            // Graceful Disconnect
            std::cout << "Connection timed out" << std::endl;
            m_ecs->queueDestroyEntity(conn->entity);
            m_connectionMap.erase(ip);
            break;
        }
    }
}


int Network::setupUDPConn(const char* address, const char* port, bool bind_sock, addrinfo** outinfo) {

    int sock;
    struct addrinfo hints, *servinfo;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    if (bind_sock)
        hints.ai_flags = AI_PASSIVE; // use my IP

    // Resolve the address and port
    if ((rv = getaddrinfo(address, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // Create a listening socket for the server on default port
    for (*outinfo = servinfo; *outinfo != nullptr; *outinfo = (*outinfo)->ai_next) {
        if ((sock = socket((*outinfo)->ai_family, (*outinfo)->ai_socktype,
                           (*outinfo)->ai_protocol)) < 0) {
            perror("socket");
            continue;
        }

        if (bind_sock && bind(sock, (*outinfo)->ai_addr, (*outinfo)->ai_addrlen) < 0) {
            close(sock);
            perror("bind");
            continue;
        }
        std::cout << "socket bound " << sock << std::endl;
        break;
    }

    if (*outinfo == nullptr) {
        fprintf(stderr, "failed to bind socket\n");
        return -2;
    }

    return sock;
}
