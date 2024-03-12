#ifndef Client_H
#define Client_H
#include "common/config.h"
#include "message/messageInfo.h"
#include "proto/msg.pb.h"

struct ClientManager{
    char *data;
    int offset;// 总偏移量
    int data_offset;// 已解包的偏移量
    std::deque<std::shared_ptr<MessageInfo>> dq;

    ClientManager();

    void add_info(const std::shared_ptr<MessageInfo>& info);

    static int char2int(const char *str);

    static void int2char(int x, char *str);

    //尝试解析出一个数据包, 若无法解析返回nullptr
    char *deserialize(int *package_len, int *package_type);

    static char *serialize(const google::protobuf::Message &msg, MessageType type, int *tot_len);

    //用于解包后删除已解包数据
    void calc_data();

    ~ClientManager();
};

#endif