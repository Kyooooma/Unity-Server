#ifndef Config_H
#define Config_H

#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <iostream>
#include <deque>
#include <memory>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>

static constexpr int DEFAULT_GAMESERVER_PORT = 8765;
static constexpr int DEFAULT_GATESERVER_PORT = 8766;
static constexpr int DEFAULT_DBSERVER_PORT = 8767;
static constexpr int MAX_CONN_LIMIT = 8196;
static constexpr int MAX_EVENTS_LIMIT = 8196;
static constexpr int BUFFER_LENGTH = 8196;
static constexpr int DATA_LENGTH = 8196;
static constexpr int EPOLL_TIMEOUT = 33;
static constexpr int EXECUTE_INTERVAL = 33;

//包头信息(长度+message类型)
static constexpr int LEN_LENGTH = 4;// len所占字节的长度, len = (LEN_LENGTH + TYPE_LENGTH + 数据包信息)
static constexpr int TYPE_LENGTH = 4;// type所占字节的长度

static std::string Log_gate = "gate.log";
static std::string Log_game = "game.log";
static std::string Log_db = "db.log";


enum ServerType{
    GateServer,
    GameServer,
    DBServer,
    DEFAULT,
};
#endif