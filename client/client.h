#ifndef UNITY_SERVER_CLIENT_H
#define UNITY_SERVER_CLIENT_H

#include "../message/messageInfo.h"
#include "../common/config.h"
#include "../message/messageUtils.h"

struct ClientManager{
    char *data;
    int offset;// 总偏移量
    int data_offset;// 已解包的偏移量
    int fd;// 客户端的fd
    std::string uid;
    std::deque<std::shared_ptr<MessageInfo>> dq;

    explicit ClientManager(int fd);

    void add_info(const std::shared_ptr<MessageInfo>& info);

    //读数据
    int read_data() const;

    //发送数据
    int send_data(const std::shared_ptr<MessageInfo>& msg) const;

    int send_all_message();

    //尝试解析出一个数据包, 若无法解析返回nullptr
    char *deserialize(int *package_len, int *package_type);

    //用于解包后删除已解包数据
    void calc_data();

    ~ClientManager();
};
#endif //UNITY_SERVER_CLIENT_H
