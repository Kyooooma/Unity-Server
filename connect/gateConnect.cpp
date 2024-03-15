#include "gateConnect.h"

GateConnectManager::GateConnectManager() {
    type = GateServer;
    is_connected = false;
    server_port = DEFAULT_GATESERVER_PORT;
    logManager.open(Log_gate);
}

void GateConnectManager::analyze_package(char *msg, MessageType msg_type, int len, std::shared_ptr<ClientManager> &cm) {
    std::shared_ptr<MessageInfo> info = nullptr;
    if(cm->fd == listen_fd){
        //说明是从gameserver发回的消息
        bool is = false, should_del = false;
        std::string uid;
        if(msg_type == MessageType::NoticeInfo_only){
            is = true;
            messagek::NoticeInfo ifo;
            MessageUtils::deserialize(ifo, msg, len);
            uid = ifo.recuser();
            if(ifo.code() == MessageCode::LogInError){
                should_del = true;
            } else if (ifo.code() == MessageCode::LogOut){
                should_del = true;
            }
            logManager.logToFile("NoticeInfo_only msg:: " + ifo.msg() + " uid:: " + ifo.recuser());
//            std::cout << "NoticeInfo_only msg:: " << ifo.msg() << " uid:: " << ifo.recuser() << "\n";
            info = MessageUtils::serialize(ifo, MessageType::NoticeInfo);
        } else if (msg_type == MessageType::SequenceNotice_only) {
            is = true;
            messagek::SequenceNotice ifo;
            MessageUtils::deserialize(ifo, msg, len);
            uid = ifo.recuser();
//            std::cout << "SequenceNotice_only\n";
            info = MessageUtils::serialize(ifo, MessageType::SequenceNotice);
        } else if (msg_type == MessageType::MoveInfo_only) {
            is = true;
            messagek::MoveInfo ifo;
            MessageUtils::deserialize(ifo, msg, len);
            uid = ifo.recuser();
//            std::cout << "MoveInfo_only\n";
            info = MessageUtils::serialize(ifo, MessageType::MoveInfo);
        } else if (msg_type == MessageType::LogInfo_only) {
            is = true;
            messagek::LogInfo ifo;
            MessageUtils::deserialize(ifo, msg, len);
            uid = ifo.recuser();
//            std::cout << "LogInfo_only\n";
            info = MessageUtils::serialize(ifo, MessageType::LogInfo);
        }else{
            info = std::make_shared<MessageInfo>(msg - LEN_LENGTH - TYPE_LENGTH, len + LEN_LENGTH + TYPE_LENGTH);
        }
        if(is){
            if(!uid2fd.count(uid)){
                std::cout << "Uid:: " << uid << " not found!!\n";
                return;
            }
            auto target = get_client(uid2fd[uid]);
            if(target == nullptr) return;
            target->add_info(info);
            if(should_del){
                del_user(uid);
            }
        }else{
            add_broadcast(info);
        }
    }else{
        info = std::make_shared<MessageInfo>(msg - LEN_LENGTH - TYPE_LENGTH, len + LEN_LENGTH + TYPE_LENGTH);
        //需要发向gameserver
        if(msg_type == MessageType::LogInfo){
            messagek::LogInfo ifo;
            MessageUtils::deserialize(ifo, msg, len);
            add_user(ifo.username(), cm->fd);
            cm->uid = ifo.username();
//            std::cout << ifo.username() << " " << ifo.password() << "\n";
        }
        auto gameserver = get_client(listen_fd);
        if(gameserver == nullptr){
            return;
        }
        gameserver->add_info(info);
    }
}

void GateConnectManager::add_user(const std::string& uid, int fd) {
    if(uid2fd.count(uid)){
        std::cout << "Uid:: " << uid << " already added!!\n";
        return;
    }
    uid2fd[uid] = fd;
}

void GateConnectManager::del_user(const std::string &uid) {
    if(!uid2fd.count(uid)){
        std::cout << "Uid:: " << uid << " not found!!\n";
        return;
    }
    int fd = uid2fd[uid];
    auto cm = get_client(fd);
    uid2fd.erase(uid);
    cm->uid.clear();
}

void GateConnectManager::close_client(int fd) {
    if(fd < 0) return;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        perror("EPOLL_CTL_DEL error.");
        return;
    }
    if (close(fd) < 0) {
        perror("Close client error.");
        return;
    }
    std::cout << "Close client fd = " << fd << "\n";
    if(fd == listen_fd) {
        listen_fd = -1;
        is_connected = false;
    } else if (fd == sock_fd){
        sock_fd = -1;
        return;
    }
    if (!fd2client.count(fd)) {
        fprintf(stderr, "Del client error: the client fd = %d has already deleted.\n", fd);
        return;
    }
    auto cm = fd2client[fd];
    send_logout(cm->uid);
    del_user(cm->uid);
    fd2client.erase(fd);
}

void GateConnectManager::send_logout(std::string uid) {
    auto listenServer = get_client(listen_fd);
    if(listenServer == nullptr) return;
    messagek::NoticeInfo info;
    info.set_msg(uid);
    info.set_code(MessageCode::LogOut);
    auto data = MessageUtils::serialize(info, MessageType::NoticeInfo);
    listenServer->add_info(data);
}
