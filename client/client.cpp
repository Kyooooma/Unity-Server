#include "client.h"

ClientManager::ClientManager() {
    //析构时delete
    data = new char[DATA_LENGTH];
    offset = data_offset = 0;
}

ClientManager::~ClientManager() {
    delete[] data;
    dq.shrink_to_fit();
}

int ClientManager::char2int(const char *str) {
    int num = str[3];
    num |= str[2] << 8;
    num |= str[1] << 16;
    num |= str[0] << 24;
    return num;
}

void ClientManager::int2char(int x, char *str) {
    str[0] = (char) (x >> 24 & 0xFF);
    str[1] = (char) (x >> 16 & 0xFF);
    str[2] = (char) (x >> 8 & 0xFF);
    str[3] = (char) (x & 0xFF);
}

char *ClientManager::deserialize(int *package_len, int *package_type) {
    if(data_offset + LEN_LENGTH > offset){
        //说明剩下数据不够一个整形
        return nullptr;
    }
    // 可以读出包头
    int len = char2int(data + data_offset);// 包总长度
    if (data_offset + len > offset) {
        //说明不足整个包
        return nullptr;
    }
    std::cout << "data_offset, len, cm_offset:: " << data_offset << ", " << len << ", " << offset << "\n";
    int type = char2int(data + data_offset + LEN_LENGTH);// 数据包类型
    data_offset = data_offset + LEN_LENGTH + TYPE_LENGTH;
    len = len - (LEN_LENGTH + TYPE_LENGTH);// 实际数据包长度
    std::cout << "read::" << len << " " << type << "\n";
    *package_len = len;
    *package_type = type;
    data_offset += len;
    return data + data_offset - len;
}

char* ClientManager::serialize(const google::protobuf::Message& msg, MessageType type, int* tot_len) {
    int len = msg.ByteSizeLong();
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

void ClientManager::calc_data() {
    if(data_offset == 0) return;
    int new_len = offset - data_offset;
    memmove(data, data + data_offset, new_len);
    offset = new_len;
    data_offset = 0;
}

void ClientManager::add_info(const std::shared_ptr<MessageInfo>& info) {
    dq.push_back(info);
}
