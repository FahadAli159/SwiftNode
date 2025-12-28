#include "NatTraversal.h"
#include "NetworkManager.h"
#include "PeerDiscovery.h"
#include "UIManager.h"
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <port> [node_name]" << std::endl;
    return -1;
  }

  int myPort = atoi(argv[1]);

  // Components (must exist for UI)
  PeerDiscovery discovery(myPort);
  NetworkManager network(myPort);
  NatTraversal nat(myPort);

  // UI Setup
  UIManager ui(myPort, &discovery, &network);
  ui.log("SwiftNode starting...");

  // Start logic
  discovery.start();
  ui.log("Peer Discovery started on port 50000");

  if (!network.start()) {
    ui.log("[ERROR] Failed to start NetworkManager (Bind failed).");
    // We don't exit here so the user can see the log in the UI
  } else {
    ui.log("Networking ready on port " + std::to_string(myPort));
  }

  // NAT Traversal
  ui.log("Determining Public IP...");
  std::string publicIP = nat.getPublicIP();
  if (!publicIP.empty()) {
    ui.setPublicIP(publicIP);
    ui.log("Public IP detected: " + publicIP);
  }

  ui.log("Attempting UPnP Port Mapping...");
  if (nat.tryMapPort()) {
    ui.log("[SUCCESS] Port " + std::to_string(myPort) + " opened via UPnP.");
  }

  // FLTK Event Loop
  ui.run();

  // Cleanup
  discovery.stop();
  network.stop();
  return 0;
}