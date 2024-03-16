#include "client.h"

ClientManager::ClientManager(int fd_) : fd(fd_) {
    //析构时delete
    data = new char[DATA_LENGTH];
    offset = data_offset = 0;
}

ClientManager::~ClientManager() {
    delete[] data;
    dq.clear();
    dq.shrink_to_fit();
}

void ClientManager::add_info(const std::shared_ptr<MessageInfo> &info) {
    dq.push_back(info);
}

int ClientManager::read_data() const {
    return recv(fd, data + offset, DATA_LENGTH - offset, 0);
}

int ClientManager::send_data(const std::shared_ptr<MessageInfo>& msg) const {
    return send(fd, msg->msg, msg->len, 0);
}

char *ClientManager::deserialize(int *package_len, int *package_type) {
    if (data_offset + LEN_LENGTH > offset) {
        //说明剩下数据不够一个整形
        return nullptr;
    }
    // 可以读出包头
    int len = MessageUtils::char2int(data + data_offset);// 包总长度
    if (data_offset + len > offset) {
        //说明不足整个包
        return nullptr;
    }
//    std::cout << "data_offset, len, cm_offset:: " << data_offset << ", " << len << ", " << offset << "\n";
    int type = MessageUtils::char2int(data + data_offset + LEN_LENGTH);// 数据包类型
    data_offset = data_offset + LEN_LENGTH + TYPE_LENGTH;
    len = len - (LEN_LENGTH + TYPE_LENGTH);// 实际数据包长度
//    std::cout << "read::" << len << " " << type << "\n";
    *package_len = len;
    *package_type = type;
    data_offset += len;
    return data + data_offset - len;
}

void ClientManager::calc_data() {
    if (data_offset == 0) return;
    int new_len = offset - data_offset;
    memmove(data, data + data_offset, new_len);
    offset = new_len;
    data_offset = 0;
}

int ClientManager::send_all_message() {
    while(!dq.empty()){
        auto msg = dq.front();
        dq.pop_front();
        if(msg == nullptr){
            std::cout << "msg is nullptr.\n";
            continue;
        }
        int ret = send_data(msg);
        if(ret < 0){
            perror("Send data error.");
            continue;
        }else if(ret == 0){
            //关闭连接
            return 0;
        }
    }
    return 1;
}
