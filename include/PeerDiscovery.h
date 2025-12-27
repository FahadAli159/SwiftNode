#pragma once
#include <set>
#include <string>
#include <mutex>
#include <atomic>
#include <thread>

class PeerDiscovery {
public:
    PeerDiscovery(int port);
    ~PeerDiscovery();
    void start();
    void stop();
    std::set<std::string> getPeers();
private:
    void broadcaster();
    void listener();
    std::thread broadcastThread;
    std::thread listenThread;
    int myPort;
    std::set<std::string> discoveredPeers;
    std::mutex peersMutex;
    std::atomic<bool> running;
};
