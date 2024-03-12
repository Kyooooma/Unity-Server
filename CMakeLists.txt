cmake_minimum_required(VERSION 3.22)
project(Unity-Server)

set(CMAKE_CXX_STANDARD 17)
include_directories(/usr/local/protobuf/include)
include_directories(${PROJECT_SOURCE_DIR})
add_subdirectory(proto)
add_subdirectory(message)
add_subdirectory(connect)
add_subdirectory(common)
add_subdirectory(client)

add_executable(server
        proto
        message
        connect
        common
        client
        message/messageUtils.h
        server/gameServer.cpp
        server/gameServer.cpp
        server/dbServer.cpp
        server/gateServer.cpp
)

