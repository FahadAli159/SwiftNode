#include "NatTraversal.h"
#include "NetworkManager.h"
#include "PeerDiscovery.h"
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char const *argv[]) {
  if (argc < 3) {
    return -1;
    // arguments less than expected
  }

  int myPort = atoi(argv[1]);

  // Peer discovery logic will be handled by PeerDiscovery class (UDP)
  PeerDiscovery discovery(myPort);
  discovery.start();

  // Attempt NAT Traversal (UPnP + Public IP)
  NatTraversal nat(myPort);
  std::cout << "Determining Public IP..." << std::endl;
  std::string publicIP = nat.getPublicIP();
  if (!publicIP.empty()) {
    std::cout << " [SUCCESS] My Public IP is: " << publicIP << std::endl;
    std::cout
        << " [INFO] Share this IP with friends to connect from the internet."
        << std::endl;
  } else {
    std::cerr << " [WARN] Could not determine Public IP." << std::endl;
  }

  std::cout << "Attempting UPnP Port Mapping..." << std::endl;
  if (nat.tryMapPort()) {
    std::cout << " [SUCCESS] Port " << myPort << " is now open via UPnP!"
              << std::endl;
  } else {
    std::cout << " [WARN] UPnP failed. You may need to manually forward port "
              << myPort << "." << std::endl;
  }

  // NetworkManager handles TCP data transfer
  NetworkManager network(myPort);
  if (!network.start()) {
    std::cerr << "Failed to start NetworkManager. Exiting." << std::endl;
    return -1;
  }

  std::cout << "SwiftNode running on port " << myPort
            << ". Press Enter to list peers or 'q' to quit." << std::endl;

  std::string input;
  while (std::getline(std::cin, input)) {
    if (input == "q")
      break;

    auto peers = discovery.getPeers();
    std::cout << "Discovered Peers: " << peers.size() << std::endl;
    for (const auto &peer : peers) {
      std::cout << " - " << peer << std::endl;
    }

    // Test connection capability
    if (input.rfind("connect ", 0) == 0) {
      // quick hack to test connection: connect <ip> <port>
      // In real app, we would parse this better
      size_t spacePos = input.find(' ', 8);
      if (spacePos != std::string::npos) {
        std::string ip = input.substr(8, spacePos - 8);
        int port = std::stoi(input.substr(spacePos + 1));
        network.connectToPeer(ip, port);
      }
    }
  }

  discovery.stop();
  network.stop();
  return 0;
}