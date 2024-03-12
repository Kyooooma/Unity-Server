#ifndef MessageManager_H
#define MessageManager_H
#include "client/client.h"
#include "callback/callback.h"

//管理收发数据
struct MessageManager {
private:
    char *buffer;
    int recv_size;
public:
    MessageManager();

    int read_data(int fd);

    void process_data(const std::shared_ptr<ClientManager>& cm);

    ~MessageManager();
};
#endif