cmake_minimum_required(VERSION 3.22)
project(Unity-Server)

include_directories(/usr/local/protobuf/include)
include_directories(${PROJECT_SOURCE_DIR})
link_directories(/usr/local/protobuf/lib)

# 设置 C++ 标准和编译选项
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2")

# 添加包含文件目录
include_directories(proto message client connect server)

# 手动添加 Protobuf 源文件
set(PROTO_SRCS proto/msg.pb.cc)

# 添加其他源文件
set(SOURCES
        message/messageInfo.cpp
        message/messageUtils.cpp
        client/client.cpp
        connect/connect.cpp
        connect/gameConnect.cpp
        connect/gateConnect.cpp
        connect/dbConnect.cpp
)

# 生成可执行文件
add_executable(game ${SOURCES} server/gameServer.cpp ${PROTO_SRCS})
target_link_libraries(game protobuf)

add_executable(gate ${SOURCES} server/gateServer.cpp ${PROTO_SRCS})
target_link_libraries(gate protobuf)

add_executable(db ${SOURCES} server/dbServer.cpp ${PROTO_SRCS})
target_link_libraries(db protobuf)