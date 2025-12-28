#pragma once
#include <atomic>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <thread>
#include <vector>

class NetworkManager {
public:
  NetworkManager(int port);
  ~NetworkManager();

  // Starts the TCP server
  bool start();
  void stop();

  // Connect to a peer node
  bool connectToPeer(const std::string &ip, int port);

  // Send data to a connected peer (placeholder for now)
  void sendData(const std::string &ip, int port, const std::string &data);

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
