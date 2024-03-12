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

char *MessageUtils::serialize(const google::protobuf::Message &msg, MessageType type, int *tot_len) {
    int len = (int)msg.ByteSizeLong();
    //在上层调用完后delete
    char *msg_byte = new char[len + LEN_LENGTH + TYPE_LENGTH];
    if(!msg.SerializeToArray(msg_byte + LEN_LENGTH + TYPE_LENGTH, len)){
        std::cout << "Failed to serialize message." << std::endl;
        return nullptr;
    }
    len += LEN_LENGTH + TYPE_LENGTH;
    int2char(len, msg_byte);
    int2char(type, msg_byte + LEN_LENGTH);
    *tot_len = len;
    return msg_byte;
}

int MessageUtils::deserialize(google::protobuf::Message &msg, char *data, int tot_len) {
    return msg.ParseFromArray(data, tot_len);
}

