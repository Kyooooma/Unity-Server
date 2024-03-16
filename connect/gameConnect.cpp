#include "gameConnect.h"

GameConnectManager::GameConnectManager() {
    type = GameServer;
    is_connected = false;
    server_port = DEFAULT_GAMESERVER_PORT;
    logManager.open(Log_game);
}

void GameConnectManager::add_frame(const std::shared_ptr<MessageInfo> &info) {//加入追帧消息
    frames.push_back(info);
}

void GameConnectManager::sync(std::shared_ptr<ClientManager> &cm, const std::string &uid) {
//    logManager.logToFile("sync:: uid::" + uid);
//    std::cout << "sync:: uid:: " << uid << "\n";
    for (const auto &msg: frames) {
        auto data = MessageUtils::set_recUser(msg, uid);
        cm->add_info(data);
    }
}

void GameConnectManager::analyze_package(char *msg, MessageType msg_type, int len, std::shared_ptr<ClientManager> &cm) {
    if (cm->fd != listen_fd) {
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
            add_broadcast(moveInfo, cm);
            add_frame(moveInfo);
        } else if (msg_type == MessageType::LogInfo) {
            messagek::LogInfo info;
            if (MessageUtils::deserialize(info, msg, len) < 0) {
                std::cout << "Failed to serialize message." << std::endl;
                return;
            }
//            std::cout << "username:: " << info.username() << " password:: " << info.password() << "\n";
            info.set_recuser(std::to_string(cm->fd));

            auto logInfo = MessageUtils::serialize(info, MessageType::LogInfo);
            auto dbserver = get_client(listen_fd);
            if (dbserver == nullptr) {
                return;
            }
            dbserver->add_info(logInfo);
        } else if(msg_type == MessageType::NoticeInfo){
            messagek::NoticeInfo info;
            if (MessageUtils::deserialize(info, msg, len) < 0) {
                std::cout << "Failed to serialize message." << std::endl;
                return;
            }
            info.set_recuser(std::to_string(cm->fd));
            auto logoutInfo = MessageUtils::serialize(info, MessageType::NoticeInfo);
            auto dbserver = get_client(listen_fd);
            if (dbserver == nullptr) {
                return;
            }
            dbserver->add_info(logoutInfo);
        }
    } else {
        //说明是从db发回的消息
        if (msg_type == MessageType::NoticeInfo) {
            messagek::NoticeInfo info;
            if (MessageUtils::deserialize(info, msg, len) < 0) {
                std::cout << "Failed to serialize message." << std::endl;
                return;
            }
            if (info.code() == MessageCode::LogInSuccess) {
                auto logInfo = MessageUtils::serialize(info, MessageType::NoticeInfo);
                //登录成功之后发送追帧消息
                auto recv = get_client(atoi(info.recuser().c_str()));
                logManager.logToFile("login success, uid:: " + (std::string)info.msg());
                if(recv == nullptr) return;
                info.set_recuser(info.msg());
                sync(recv, info.msg());
                add_user(info.msg(), recv->fd);
                //添加广播
                add_broadcast(logInfo, recv);
                add_frame(logInfo);
            }else if (info.code() == MessageCode::LogInError){
                logManager.logToFile("login error, uid:: " + (std::string)info.msg());
                auto recv = get_client(atoi(info.recuser().c_str()));
                if(recv == nullptr) return;
                info.set_recuser(info.msg());
                auto logerrorInfo = MessageUtils::serialize(info, MessageType::NoticeInfo_only);
                recv->add_info(logerrorInfo);
            }else if(info.code() == MessageCode::LogOut){
                logManager.logToFile("log out, uid:: " + (std::string)info.msg());
                auto recv = get_client(atoi(info.recuser().c_str()));
                if(recv == nullptr) return;
                info.set_recuser(info.msg());
                auto logInfo = MessageUtils::serialize(info, MessageType::NoticeInfo);
                //添加广播
                add_broadcast(logInfo, recv);
                del_user(info.msg(), recv->fd);
            } else if(info.code() == MessageCode::ConnectSuccess){
                //
            }
        }
    }
}

void GameConnectManager::add_broadcast(const std::shared_ptr<MessageInfo> &info, std::shared_ptr<ClientManager> &cm) {
    if(!gate_fd2log_uid.count(cm->fd)){
        std::cout << "gate_fd not found!!\n";
        return;
    }
    auto log_uid = gate_fd2log_uid[cm->fd];
    for(const auto& uid : log_uid){
        auto data = MessageUtils::set_recUser(info, uid);
        cm->add_info(data);
    }
}

void GameConnectManager::add_user(const std::string& user, int gate_fd) {
    if(!gate_fd2log_uid.count(gate_fd)){
        std::cout << "gate_fd not found!!\n";
        return;
    }
    gate_fd2log_uid[gate_fd].insert(user);
}

void GameConnectManager::del_user(const std::string &user, int gate_fd) {
    if(!gate_fd2log_uid.count(gate_fd)){
        std::cout << "gate_fd not found!!\n";
        return;
    }
    gate_fd2log_uid[gate_fd].erase(user);
}

void GameConnectManager::add_client(int fd) {
    ConnectManager::add_client(fd);
    if (gate_fd2log_uid.count(fd)) {
        fprintf(stderr, "Add client error: the client fd = %d has already added.\n", fd);
        return;
    }
    gate_fd2log_uid[fd] = std::unordered_set<std::string>();
}

void GameConnectManager::close_client(int fd) {
    ConnectManager::close_client(fd);
    if (!gate_fd2log_uid.count(fd)) {
        fprintf(stderr, "Del client error: the client fd = %d has already deleted.\n", fd);
        return;
    }
    gate_fd2log_uid.erase(fd);
}

void GameConnectManager::add_broadcast(const std::shared_ptr<MessageInfo> &info) {
    for(auto &[fd, cm] : fd2client){
        if(fd == listen_fd) continue;
        add_broadcast(info, cm);
    }
}

