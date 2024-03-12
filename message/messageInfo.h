#ifndef MessageInfo_H
#define MessageInfo_H
#include <cstring>

enum MessageType {
    MoveInfo,
    LogInfo,
    SequenceNotice,
    COUNT
};

//序列化完成的帧数据
struct MessageInfo {
    char *msg;
    int len;

    MessageInfo(char *val, int length);

    ~MessageInfo();
};
#endif