#ifndef UNITY_SERVER_MESSAGEUTILS_H
#define UNITY_SERVER_MESSAGEUTILS_H
#include "messageInfo.h"
#include "../common/config.h"
#include "proto/msg.pb.h"

struct MessageUtils{
    static int char2int(const char *str);

    static void int2char(int x, char *str);

    //记得delete char*
    static char *serialize(const google::protobuf::Message &msg, MessageType type, int *tot_len);

    static int deserialize(google::protobuf::Message &msg, char *data, int tot_len);

};
#endif //UNITY_SERVER_MESSAGEUTILS_H
