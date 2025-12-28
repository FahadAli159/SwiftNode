#include "UIManager.h"
#include "NetworkManager.h"
#include "PeerDiscovery.h"
#include <iostream>

UIManager::UIManager(int port, PeerDiscovery *pd, NetworkManager *nm)
    : peerDiscovery(pd), networkManager(nm) {
  window = new Fl_Window(650, 480, "SwiftNode P2P Client");

  // Header Section
  Fl_Box *title = new Fl_Box(20, 10, 610, 40, "SWIFTNODE DASHBOARD");
  title->labelsize(20);
  title->labelfont(FL_BOLD);

  ipBox = new Fl_Box(20, 60, 300, 25, "Public IP: Fetching...");
  ipBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

  std::string portStr = "Local Port: " + std::to_string(port);
  portBox = new Fl_Box(330, 60, 300, 25, "Local Port: ...");
  portBox->copy_label(portStr.c_str());
  portBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

  // Peers Section
  new Fl_Box(20, 100, 300, 20, "Connected Peers (LAN):");
  peerBrowser = new Fl_Browser(20, 120, 300, 150);

  // Connection Section
  connectInput = new Fl_Input(20, 300, 220, 25, "Connect to Peer (IP:Port):");
  connectInput->align(FL_ALIGN_TOP_LEFT);
  connectBtn = new Fl_Button(250, 300, 70, 25, "Join");
  connectBtn->callback(connect_cb, this);

  // Logs Section
  new Fl_Box(340, 100, 290, 20, "System Logs:");
  logBrowser = new Fl_Browser(340, 120, 290, 340);
  logBrowser->type(FL_MULTI_BROWSER);
  logBrowser->textfont(FL_COURIER);
  logBrowser->textsize(12);

  window->end();
  window->callback(quit_cb, this);

  // Setup update timer
  Fl::add_timeout(1.0, update_timer_cb, this);
}

void UIManager::run() {
  window->show();
  Fl::run();
}

void UIManager::setPublicIP(const std::string &ip) {
  std::string label = "Public IP: " + ip;
  ipBox->copy_label(label.c_str());
}

void UIManager::log(const std::string &message) {
  logBrowser->add(message.c_str());
  logBrowser->bottomline(logBrowser->size());
}

void UIManager::connect_cb(Fl_Widget *w, void *data) {
  UIManager *ui = (UIManager *)data;
  std::string input = ui->connectInput->value();
  if (input.empty())
    return;

  size_t colonPos = input.find(':');
  if (colonPos != std::string::npos) {
    std::string ip = input.substr(0, colonPos);
    int port = std::stoi(input.substr(colonPos + 1));
    ui->log("Connecting to " + ip + ":" + std::to_string(port) + "...");
    ui->networkManager->connectToPeer(ip, port);
  } else {
    ui->log("[ERR] Invalid format. Use IP:Port");
  }
}

void UIManager::update_timer_cb(void *data) {
  UIManager *ui = (UIManager *)data;
  if (ui->peerDiscovery) {
    auto peersSet = ui->peerDiscovery->getPeers();
    ui->peerBrowser->clear();
    for (const auto &peer : peersSet) {
      ui->peerBrowser->add(peer.c_str());
    }
  }
  Fl::repeat_timeout(2.0, update_timer_cb, data);
}

void UIManager::quit_cb(Fl_Widget *w, void *data) {
  std::cout << "Exiting application..." << std::endl;
  exit(0);
}
