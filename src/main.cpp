#include <iostream>
#include <cstdlib>
#include <string>
#include "PeerDiscovery.h"
#include "NetworkManager.h"

int main(int argc, char const *argv[]) {
    if(argc < 3 ) {
        return -1;
        // arguments less than expected 
    }

    int myPort = atoi(argv[1]);

    // Peer discovery logic will be handled by PeerDiscovery class (UDP)
    PeerDiscovery discovery(myPort);
    discovery.start();

    // NetworkManager handles TCP data transfer
    NetworkManager network(myPort);
    network.start();

    std::cout << "SwiftNode running on port " << myPort << ". Press Enter to list peers or 'q' to quit." << std::endl;

    std::string input;
    while (std::getline(std::cin, input)) {
        if (input == "q") break;
        
        auto peers = discovery.getPeers();
        std::cout << "Discovered Peers: " << peers.size() << std::endl;
        for (const auto& peer : peers) {
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