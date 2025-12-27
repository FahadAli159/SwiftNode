#pragma once
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <netinet/in.h>
#include <atomic>

class NetworkManager {
public:
    NetworkManager(int port);
    ~NetworkManager();

    void start();
    void stop();
    
    // Connect to a peer node
    bool connectToPeer(const std::string& ip, int port);
    
    // Send data to a connected peer (placeholder for now)
    void sendData(const std::string& ip, int port, const std::string& data);

private:
    void acceptLoop();
    void handleClient(int clientSocket);

    int serverSocket;
    int myPort;
    std::atomic<bool> running;
    std::thread acceptThread;
    std::vector<std::thread> clientThreads;
    std::mutex threadsMutex;
};
