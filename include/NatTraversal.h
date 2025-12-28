#pragma once
#include <string>

class NatTraversal {
public:
    NatTraversal(int port);
    ~NatTraversal();

    // Fetches public IP using an external HTTP service
    std::string getPublicIP();

    // Attempts to map the local port to the external router using UPnP
    bool tryMapPort();

private:
    int localPort;
};
