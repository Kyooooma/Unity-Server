#include "message.h"

MessageManager::MessageManager() {
    //析构时delete
    buffer = new char[BUFFER_LENGTH];
    recv_size = 0;
}

MessageManager::~MessageManager() {
    delete[] buffer;
}

int MessageManager::read_data(int fd) {
    recv_size = recv(fd, buffer, BUFFER_LENGTH, 0);
    return recv_size;
}

void MessageManager::process_data(const std::shared_ptr<ClientManager>& cm) {
    //处理缓冲区数据
    int offset = 0;
    while (offset < recv_size) {
        //将缓冲区数据拼接进入data中
        int size = std::min(recv_size - offset, DATA_LENGTH - cm->offset);
        std::cout << "recv_size:: " << size << "\n";
        memcpy(cm->data + cm->offset, buffer + offset, size);
        offset += size;
        cm->offset += size;
        //处理data, 尝试解包
        int len, type;//数据包长度和类型
        while(true){
            //读取一个数据包
            auto package = cm->deserialize(&len, &type);
            if(package == nullptr){
                //说明数据包解析完成, 将处理完的数据包删除
                cm->calc_data();
                break;
            }

            CallBack::getInstance().TriggerCallback((MessageType)type, package, len);
        }
    }
}


