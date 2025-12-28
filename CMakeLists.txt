cmake_minimum_required(VERSION 3.10)
project(SwiftNode)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)


find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

add_executable(SwiftNode src/main.cpp src/PeerDiscovery.cpp src/NetworkManager.cpp src/NatTraversal.cpp)
target_link_libraries(SwiftNode Threads::Threads ${CURL_LIBRARIES} miniupnpc)
