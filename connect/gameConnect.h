#ifndef UNITY_SERVER_GAMECONNECT_H
#define UNITY_SERVER_GAMECONNECT_H
#include "connect.h"
struct GameConnectManager : ConnectManager{
    //游戏开始的所有帧
    std::vector<std::shared_ptr<MessageInfo>> frames;
    std::map<int, std::unordered_set<std::string>> gate_fd2log_uid;

    GameConnectManager();

    void analyze_package(char * msg, MessageType msg_type, int len, std::shared_ptr<ClientManager>& cm) override; // 解析proto包

    void add_frame(const std::shared_ptr<MessageInfo>& info);

    void sync(std::shared_ptr<ClientManager>& cm, const std::string& uid); //追帧

    void add_broadcast(const std::shared_ptr<MessageInfo>& info, std::shared_ptr<ClientManager> &cm); //添加广播事件

    void add_broadcast(const std::shared_ptr<MessageInfo>& info); //添加广播事件

    void add_user(const std::string& user, int gate_fd);

    void del_user(const std::string& user, int gate_fd);

    void add_client(int fd) override;

    void close_client(int fd) override;

    void add_info_only();
};
#endif //UNITY_SERVER_GAMECONNECT_H
