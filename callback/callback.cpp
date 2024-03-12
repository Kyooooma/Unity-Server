#include "callback.h"

void CallBack::AddCallback(MessageType type, std::function<void(char *, int)> callback) {
    if(callbackTable.count(type)){
        std::cout << "There has already a callback for type = " << type << std::endl;
        return;
    }
    callbackTable[type] = std::move(callback);
    std::cout << "Add a callback for type = " << type << " success." << std::endl;
}

void CallBack::TriggerCallback(MessageType type, char *data, int len) {
    if(callbackTable.count(type)){
        std::cout << "type:: " << type << "\n";
        callbackTable[type](data, len);
    }else{
        std::cout << "Callback not found for type: " << type << std::endl;
    }
}




