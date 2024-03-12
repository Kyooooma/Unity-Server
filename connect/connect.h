#ifndef Connect_H
#define Connect_H
#include "message/message.h"

struct ConnectManager {
private:
    std::map<int, std::shared_ptr<ClientManager>> fd2client;
    std::vector<std::shared_ptr<MessageInfo>> frames;
    std::unique_ptr<MessageManager> messageManager;

    static void SetNONBLOCK(int fd); //设置成非阻塞

    void add_client(int fd); //新建一个客户端信息

    void sync(int fd);
public:
    int sock_fd{}, epoll_fd{};

    ConnectManager();

    void startServer(int port); //启动服务器

    void handle_accept(); //接受新的客户端连接

    void handle_read(int fd); //接受客户端的数据

    void close_client(int fd); //关闭客户端连接

    std::shared_ptr<ClientManager> get_client(int fd);

    void add_broadcast(char *val, int length);

    void broadcast(int curFrame); //广播

    // 获取单例实例的静态方法
    static ConnectManager& getInstance() {
        // 在第一次使用时创建实例
        static ConnectManager instance = ConnectManager();
        return instance;
    }
};
#endif