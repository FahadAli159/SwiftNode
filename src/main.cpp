#include <iostream>
#include <cstdlib>

int main(int argc, char const *argv[]) {
    if(argc < 3 ) {
        return -1;
        // arguments less than expected 
    }

    int myPort = atoi(argv[1]);

    // Peer discovery logic will be handled by PeerDiscovery class

    std::cout << "Main program running. Add PeerDiscovery usage here." << std::endl;
    return 0;
}