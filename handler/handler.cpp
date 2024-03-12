#include "handler.h"

void Handler::handle_moveinfo(char *data, int len) {
    messagek::MoveInfo info;
    if(!info.ParseFromArray(data, len)){
        printf("Deserialize to Moveinfo error.");
        return;
    }
    std::cout << "uid:: " << info.uid() << " horizontal:: " << info.horizontal() << " vertical:: "
              << info.vertical() << " is_atk:: " << info.is_atk() << "\n";

    //进行序列化
    int tot_len;
    //new
    auto msg = ClientManager::serialize(info, MessageType::MoveInfo, &tot_len);
    if(msg == nullptr){
        return;
    }
    //广播
    ConnectManager::getInstance().add_broadcast(msg, tot_len);
    delete[] msg;
}

void Handler::handle_loginfo(char *data, int len) {
    messagek::LogInfo info;
    if(!info.ParseFromArray(data, len)){
        printf("Deserialize to Moveinfo error.");
        return;
    }
    std::cout << "username:: " <<  info.username() << " password:: " << info.password() << "\n";
    //进行序列化
    info.set_password("SUCCESS");
    int tot_len;
    //new
    auto msg = ClientManager::serialize(info, MessageType::LogInfo, &tot_len);
    if(msg == nullptr){
        return;
    }
    //广播
    ConnectManager::getInstance().add_broadcast(msg, tot_len);
    delete[] msg;
}
