#include "PeerDiscovery.h"
#include <thread>
#include <iostream>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

PeerDiscovery::PeerDiscovery(int port)
    : myPort(port), running(false) {}

PeerDiscovery::~PeerDiscovery() {
    stop();
}

void PeerDiscovery::start() {
    running = true;
    broadcastThread = std::thread(&PeerDiscovery::broadcaster, this);
    listenThread = std::thread(&PeerDiscovery::listener, this);
}

void PeerDiscovery::stop() {
    running = false;
    if (broadcastThread.joinable()) broadcastThread.join();
    if (listenThread.joinable()) listenThread.join();
}

std::set<std::string> PeerDiscovery::getPeers() {
    std::lock_guard<std::mutex> lock(peersMutex);
    return discoveredPeers;
}

void PeerDiscovery::broadcaster() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create broadcast socket" << std::endl;
        return;
    }
    int broadcastEnable = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    sockaddr_in broadcastAddr{};
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(50000);
    broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    char msg[64];
    snprintf(msg, sizeof(msg), "PEER:%d", myPort);
    while (running) {
        sendto(sock, msg, strlen(msg), 0, (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    close(sock);
}

void PeerDiscovery::listener() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create listener socket" << std::endl;
        return;
    }
    sockaddr_in listenAddr{};
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_port = htons(50000);
    listenAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (sockaddr*)&listenAddr, sizeof(listenAddr)) < 0) {
        std::cerr << "Failed to bind listener socket" << std::endl;
        close(sock);
        return;
    }
    char buf[128];
    while (running) {
        sockaddr_in senderAddr{};
        socklen_t senderLen = sizeof(senderAddr);
        int len = recvfrom(sock, buf, sizeof(buf)-1, 0, (sockaddr*)&senderAddr, &senderLen);
        if (len > 0) {
            buf[len] = '\0';
            std::string msg(buf);
            if (msg.find("PEER:") == 0) {
                int peerPort = atoi(msg.substr(5).c_str());
                std::string peerIp = inet_ntoa(senderAddr.sin_addr);
                if (peerPort != myPort) {
                    std::lock_guard<std::mutex> lock(peersMutex);
                    discoveredPeers.insert(peerIp + ":" + std::to_string(peerPort));
                    std::cout << "Discovered peer: " << peerIp << ":" << peerPort << std::endl;
                }
            }
        }
    }
    close(sock);
}
