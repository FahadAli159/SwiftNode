#include "NatTraversal.h"
#include <cstring>
#include <curl/curl.h>
#include <iostream>
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>
#include <vector>

// Callback for writing CURL data
static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

NatTraversal::NatTraversal(int port) : localPort(port) {}

NatTraversal::~NatTraversal() {}

std::string NatTraversal::getPublicIP() {
  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); // 5 second timeout
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res)
                << std::endl;
    }
    curl_easy_cleanup(curl);
  }
  return readBuffer;
}

bool NatTraversal::tryMapPort() {
  int error = 0;
  struct UPNPDev *devlist = upnpDiscover(10000, NULL, NULL, 0, 0, 2, &error);
  if (!devlist) {
    std::cout << "No UPnP devices discovered." << std::endl;
    return false;
  }

  struct UPNPUrls urls;
  struct IGDdatas data;
  char lanaddr[64];
  char wanaddr[64];

  int status = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr),
                                wanaddr, sizeof(wanaddr));
  if (status != 1) {
    std::cout << "No valid Internet Gateway Device found." << std::endl;
    freeUPNPDevlist(devlist);
    return false;
  }

  std::cout << "Found IGD: " << urls.controlURL << " (Local LAN IP: " << lanaddr
            << ")" << std::endl;

  // Check if mapping already exists or add it
  std::string portStr = std::to_string(localPort);
  const char *internalClient = lanaddr;

  // AddPortMapping(urls, data, external_port, protocol, internal_client,
  // internal_port, desc, remote_host, lease_duration)
  int r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,
                              portStr.c_str(), "TCP", internalClient,
                              portStr.c_str(), "SwiftNode P2P", NULL, "604800");

  if (r != UPNPCOMMAND_SUCCESS) {
    std::cout << "AddPortMapping failed with error code " << r << " ("
              << strupnperror(r) << ")" << std::endl;
    FreeUPNPUrls(&urls);
    freeUPNPDevlist(devlist);

    // Fallback: Try using the upnpc command line tool if installed
    std::cout << " [INFO] Attempting fallback to 'upnpc' system command..."
              << std::endl;
    std::string command = "upnpc -a " + std::string(lanaddr) + " " + portStr +
                          " " + portStr + " TCP";
    int result = system(command.c_str());

    if (result == 0) {
      std::cout << " [SUCCESS] UPnP Fallback successful via upnpc CLI!"
                << std::endl;
      return true;
    }

    return false;
  }

  std::cout << "UPnP Port Mapping Successful! Port " << localPort
            << " is now forwarded." << std::endl;

  FreeUPNPUrls(&urls);
  freeUPNPDevlist(devlist);
  return true;
}
