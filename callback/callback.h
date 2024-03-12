#ifndef Callback_H
#define Callback_H
#include <map>
#include <functional>
#include <iostream>
#include "message/messageInfo.h"

struct CallBack {
public:
    // 获取单例实例的静态方法
    static CallBack& getInstance() {
        // 在第一次使用时创建实例
        static CallBack instance;
        return instance;
    }

    // 添加回调函数到回调表
    void AddCallback(MessageType type, std::function<void(char *, int)> callback);

    // 触发回调函数
    void TriggerCallback(MessageType type, char * data, int len);

private:
    // 私有构造函数，防止外部直接实例化
    CallBack(){}
    // 禁用拷贝构造函数和赋值操作符，确保只有一个实例
    CallBack(const CallBack&) = delete;
    CallBack& operator=(const CallBack&) = delete;

    std::map<MessageType, std::function<void(char *, int)>> callbackTable;
};
#endif