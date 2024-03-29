#ifndef UNITY_SERVER_GATECONNECT_H
#define UNITY_SERVER_GATECONNECT_H
#include "connect.h"
struct GateConnectManager : ConnectManager{
    std::unordered_map<std::string, int> uid2fd;

    GateConnectManager();

    void analyze_package(char * msg, MessageType msg_type, int len, std::shared_ptr<ClientManager>& cm) override; // 解析proto包

    void add_user(const std::string& uid, int fd);

    void del_user(const std::string & uid);

    void close_client(int fd) override;

    void send_logout(std::string uid);
};
#endif //UNITY_SERVER_GATECONNECT_H
