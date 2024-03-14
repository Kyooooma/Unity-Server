#ifndef UNITY_SERVER_MESSAGEUTILS_H
#define UNITY_SERVER_MESSAGEUTILS_H
#include "messageInfo.h"
#include "../common/config.h"
#include "proto/msg.pb.h"

struct MessageUtils{
    static int char2int(const char *str);

    static void int2char(int x, char *str);

    static std::shared_ptr<MessageInfo> serialize(const google::protobuf::Message &msg, MessageType type);

    static int deserialize(google::protobuf::Message &msg, char *data, int tot_len);

    static std::shared_ptr<MessageInfo> set_recUser(const std::shared_ptr<MessageInfo>& info, std::string uid);

};
#endif //UNITY_SERVER_MESSAGEUTILS_H
