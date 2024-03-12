#include "messageInfo.h"

MessageInfo::MessageInfo(char *val, int length) {
    //析构时delete
    msg = new char[length];
    memcpy(msg, val, length);
    len = length;
}

MessageInfo::~MessageInfo() {
    delete[] msg;
}