#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Window.H>
#include <string>
#include <vector>

class PeerDiscovery;
class NetworkManager;

class UIManager {
public:
  UIManager(int port, PeerDiscovery *pd, NetworkManager *nm);
  void run();

  // UI Updates
  void setPublicIP(const std::string &ip);
  void updatePeers(const std::vector<std::string> &peers);
  void log(const std::string &message);

private:
  static void connect_cb(Fl_Widget *w, void *data);
  static void update_timer_cb(void *data);
  static void quit_cb(Fl_Widget *w, void *data);

  PeerDiscovery *peerDiscovery;
  NetworkManager *networkManager;

  Fl_Window *window;
  Fl_Box *ipBox;
  Fl_Box *portBox;
  Fl_Browser *peerBrowser;
  Fl_Browser *logBrowser;
  Fl_Input *connectInput;
  Fl_Button *connectBtn;
};
