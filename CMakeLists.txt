cmake_minimum_required(VERSION 3.10)
project(SwiftNode)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)


find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(FLTK REQUIRED)

add_executable(SwiftNode src/main.cpp src/PeerDiscovery.cpp src/NetworkManager.cpp src/NatTraversal.cpp src/StorageManager.cpp src/UIManager.cpp)
target_link_libraries(SwiftNode Threads::Threads ${CURL_LIBRARIES} miniupnpc OpenSSL::SSL OpenSSL::Crypto ${FLTK_LIBRARIES})
