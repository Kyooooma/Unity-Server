#include "gameConnect.h"

GameConnectManager::GameConnectManager() {
    type = GameServer;
    server_port = DEFAULT_GAMESERVER_PORT;
}

void GameConnectManager::add_frame(const std::shared_ptr<MessageInfo> &info) {//加入追帧消息
    frames.push_back(info);
}

void GameConnectManager::sync(std::shared_ptr<ClientManager>& cm, const std::string& uid) {
    std::cout << "sync:: uid:: " << uid << "\n";
    for (const auto &msg: frames) {
        auto data = MessageUtils::set_recUser(msg, uid);
        cm->add_info(data);
    }
}

void GameConnectManager::analyze_package(char *msg, MessageType msg_type, int len, std::shared_ptr<ClientManager> &cm) {
    if(cm->fd != listen_fd) {
        if (msg_type == MessageType::MoveInfo) {
            messagek::MoveInfo info;
            if (MessageUtils::deserialize(info, msg, len) < 0) {
                std::cout << "Failed to serialize message." << std::endl;
                return;
            }
            std::cout << "uid:: " << info.uid() << " horizontal:: " << info.horizontal() << " vertical:: "
                      << info.vertical() << " is_atk:: " << info.is_atk() << "\n";
            auto moveInfo = MessageUtils::serialize(info, msg_type);
            //添加广播
            add_broadcast(moveInfo);
            add_frame(moveInfo);
        } else if (msg_type == MessageType::LogInfo) {
            messagek::LogInfo info;
            if (MessageUtils::deserialize(info, msg, len) < 0) {
                std::cout << "Failed to serialize message." << std::endl;
                return;
            }
            std::cout << "username:: " << info.username() << " password:: " << info.password() << "\n";
            //发送给dbserver  tbd

            messagek::NoticeInfo noticeInfo;
            noticeInfo.set_recuser(info.username());
            noticeInfo.set_code(200);
            noticeInfo.set_msg(info.username());
            auto logInfo = MessageUtils::serialize(noticeInfo, MessageType::NoticeInfo);
            //登录成功之后发送追帧消息
            sync(cm, info.username());
            //添加广播
            add_broadcast(logInfo);
            add_frame(logInfo);
        }
    }
}

