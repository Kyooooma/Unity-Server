#include "messageUtils.h"

void MessageUtils::int2char(int x, char *str) {
    str[0] = (char) (x >> 24 & 0xFF);
    str[1] = (char) (x >> 16 & 0xFF);
    str[2] = (char) (x >> 8 & 0xFF);
    str[3] = (char) (x & 0xFF);
}

int MessageUtils::char2int(const char *str) {
    int num = str[3];
    num |= str[2] << 8;
    num |= str[1] << 16;
    num |= str[0] << 24;
    return num;
}

std::shared_ptr<MessageInfo> MessageUtils::serialize(const google::protobuf::Message &msg, MessageType type) {
    int len = (int)msg.ByteSizeLong();
    char *msg_byte = new char[len + LEN_LENGTH + TYPE_LENGTH];
    if(!msg.SerializeToArray(msg_byte + LEN_LENGTH + TYPE_LENGTH, len)){
        std::cout << "Failed to serialize message." << std::endl;
        return nullptr;
    }
    len += LEN_LENGTH + TYPE_LENGTH;
    int2char(len, msg_byte);
    int2char(type, msg_byte + LEN_LENGTH);
    auto info = std::make_shared<MessageInfo>(msg_byte, len);
    delete[] msg_byte;
    return info;
}

int MessageUtils::deserialize(google::protobuf::Message &msg, char *data, int tot_len) {
    return msg.ParseFromArray(data, tot_len);
}

std::shared_ptr<MessageInfo> MessageUtils::set_recUser(const std::shared_ptr<MessageInfo>& info, std::string uid) {
    int len = MessageUtils::char2int(info->msg);// 包总长度
    int type = MessageUtils::char2int(info->msg + LEN_LENGTH);// 数据包类型
    len = len - (LEN_LENGTH + TYPE_LENGTH);// 实际数据包长度
    if(type == MessageType::MoveInfo){
        messagek::MoveInfo ifo;
        deserialize(ifo, info->msg + LEN_LENGTH + TYPE_LENGTH, len);
        ifo.set_recuser(uid);
        auto ret = serialize(ifo, MessageType::MoveInfo_only);
        return ret;
    } else if(type == MessageType::LogInfo){
        messagek::LogInfo ifo;
        deserialize(ifo, info->msg + LEN_LENGTH + TYPE_LENGTH, len);
        ifo.set_recuser(uid);
        auto ret = serialize(ifo, MessageType::LogInfo_only);
        return ret;
    } else if(type == MessageType::SequenceNotice){
        messagek::SequenceNotice ifo;
        deserialize(ifo, info->msg + LEN_LENGTH + TYPE_LENGTH, len);
        ifo.set_recuser(uid);
        auto ret = serialize(ifo, MessageType::SequenceNotice_only);
        return ret;
    } else if(type == MessageType::NoticeInfo){
        messagek::NoticeInfo ifo;
        deserialize(ifo, info->msg + LEN_LENGTH + TYPE_LENGTH, len);
        ifo.set_recuser(uid);
        auto ret = serialize(ifo, MessageType::NoticeInfo_only);
        return ret;
    }else{
        std::cout << type << "\n";
    }
    return nullptr;
}

