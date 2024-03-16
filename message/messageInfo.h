#ifndef UNITY_SERVER_MESSAGEINFO_H
#define UNITY_SERVER_MESSAGEINFO_H
#include <cstring>

enum MessageType {
    MoveInfo,
    LogInfo,
    SequenceNotice,
    NoticeInfo,
    MoveInfo_only,
    LogInfo_only,
    SequenceNotice_only,
    NoticeInfo_only,
    COUNT
};

enum MessageCode{
    LogInSuccess = 200,
    LogInError = 403,
    LogOut = 114,
    ConnectSuccess
};

//序列化完成的帧数据
struct MessageInfo {
    char *msg;
    int len;

    MessageInfo(char *val, int length);

    ~MessageInfo();
};
#endif //UNITY_SERVER_MESSAGEINFO_H
