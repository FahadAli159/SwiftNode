#include "NetworkManager.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

NetworkManager::NetworkManager(int port) : myPort(port), running(false), serverSocket(-1) {}

NetworkManager::~NetworkManager() {
    stop();
}

void NetworkManager::start() {
    if (running) return;
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Failed to create TCP socket" << std::endl;
        return;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(myPort);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind TCP socket on port " << myPort << std::endl;
        close(serverSocket);
        return;
    }

    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Failed to listen on TCP socket" << std::endl;
        close(serverSocket);
        return;
    }

    running = true;
    std::cout << "NetworkManager started on port " << myPort << std::endl;
    acceptThread = std::thread(&NetworkManager::acceptLoop, this);
}

void NetworkManager::stop() {
    if (!running) return;
    running = false;

    // Close socket to wake up accept thread
    if (serverSocket >= 0) {
        close(serverSocket);
        serverSocket = -1;
    }

    if (acceptThread.joinable()) {
        acceptThread.join();
    }

    std::lock_guard<std::mutex> lock(threadsMutex);
    for (auto& t : clientThreads) {
        if (t.joinable()) t.join();
    }
    clientThreads.clear();
}

void NetworkManager::acceptLoop() {
    while (running) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);

        if (clientSocket < 0) {
            if (running) {
                std::cerr << "Accept failed" << std::endl;
            }
            continue;
        }

        std::cout << "New connection from " << inet_ntoa(clientAddr.sin_addr) << std::endl;
        
        std::lock_guard<std::mutex> lock(threadsMutex);
        clientThreads.emplace_back(&NetworkManager::handleClient, this, clientSocket);
    }
}

void NetworkManager::handleClient(int clientSocket) {
    char buffer[1024];
    while (running) {
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            break; // Connection closed or error
        }
        buffer[bytesRead] = '\0';
        std::cout << "Received: " << buffer << std::endl;
        
        // Echo back for now
        send(clientSocket, "ACK: ", 5, 0);
        send(clientSocket, buffer, bytesRead, 0);
    }
    close(clientSocket);
}

bool NetworkManager::connectToPeer(const std::string& ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;

    sockaddr_in peerAddr{};
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &peerAddr.sin_addr);

    if (connect(sock, (sockaddr*)&peerAddr, sizeof(peerAddr)) < 0) {
        close(sock);
        return false;
    }

    std::cout << "Connected to peer " << ip << ":" << port << std::endl;
    
    // For now just send a greeting
    std::string greeting = "Hello from " + std::to_string(myPort);
    send(sock, greeting.c_str(), greeting.length(), 0);
    close(sock); // Close for now, real implementation would keep it open
    return true;
}

void NetworkManager::sendData(const std::string& ip, int port, const std::string& data) {
    // Placeholder - reuse connect logic or find existing connection
    connectToPeer(ip, port);
}
