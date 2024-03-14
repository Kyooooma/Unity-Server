#ifndef UNITY_SERVER_GAMECONNECT_H
#define UNITY_SERVER_GAMECONNECT_H
#include "connect.h"
struct GameConnectManager : ConnectManager{
    //游戏开始的所有帧
    std::vector<std::shared_ptr<MessageInfo>> frames;

    GameConnectManager();

    void analyze_package(char * msg, MessageType msg_type, int len, std::shared_ptr<ClientManager>& cm) override; // 解析proto包

    void add_frame(const std::shared_ptr<MessageInfo>& info);

    void sync(std::shared_ptr<ClientManager>& cm, const std::string& uid); //追帧

};
#endif //UNITY_SERVER_GAMECONNECT_H
