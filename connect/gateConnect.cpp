#include "gateConnect.h"

GateConnectManager::GateConnectManager() {
    type = GateServer;
    is_connected = false;
    server_port = DEFAULT_GATESERVER_PORT;
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
            }
            std::cout << "NoticeInfo_only msg:: " << ifo.msg() << " uid:: " << ifo.recuser() << "\n";
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
            std::cout << ifo.username() << " " << ifo.password() << "\n";
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
    uid2fd.erase(uid);
}
