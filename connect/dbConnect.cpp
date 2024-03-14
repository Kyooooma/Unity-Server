#include "dbConnect.h"

DBConnectManager::DBConnectManager() {
    type = DBServer;
    server_port = DEFAULT_DBSERVER_PORT;
}

void DBConnectManager::analyze_package(char *msg, MessageType msg_type, int len, std::shared_ptr<ClientManager> &cm) {
    if (msg_type == MessageType::LogInfo) {
        messagek::LogInfo info;
        if (MessageUtils::deserialize(info, msg, len) < 0) {
            std::cout << "Failed to serialize message." << std::endl;
            return;
        }
        std::cout << "username:: " << info.username() << " password:: " << info.password() << "\n";
        //校验tbd
        auto ok = db.login(info.username(), info.password());
        messagek::NoticeInfo noticeInfo;
        noticeInfo.set_recuser(info.recuser());
        noticeInfo.set_msg(info.username());
        if(ok){
            //login success
            std::cout << "login success.\n";
            noticeInfo.set_code(MessageCode::LogInSuccess);
        }else{
            //login error
            noticeInfo.set_code(MessageCode::LogInError);
        }
        auto logInfo = MessageUtils::serialize(noticeInfo, MessageType::NoticeInfo);
        cm->add_info(logInfo);
    }else if(msg_type == MessageType::NoticeInfo){
        messagek::NoticeInfo info;
        if (MessageUtils::deserialize(info, msg, len) < 0) {
            std::cout << "Failed to serialize message." << std::endl;
            return;
        }
        if(info.code() == MessageCode::LogOut){
            db.logout(info.msg());
        }
        auto logoutInfo = MessageUtils::serialize(info, MessageType::NoticeInfo);
        cm->add_info(logoutInfo);
    }
}
