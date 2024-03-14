#ifndef UNITY_SERVER_DBCONNECT_H
#define UNITY_SERVER_DBCONNECT_H
#include "connect.h"
#include "dbUtils.h"

struct DBConnectManager : ConnectManager{
    databaseManager db{};

    DBConnectManager();

    void analyze_package(char * msg, MessageType msg_type, int len, std::shared_ptr<ClientManager>& cm) override; // 解析proto包

};
#endif //UNITY_SERVER_DBCONNECT_H
