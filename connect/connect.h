#ifndef UNITY_SERVER_CONNECT_H
#define UNITY_SERVER_CONNECT_H

#include "../common/config.h"
#include "unordered_map"
#include "../client/client.h"
#include "vector"

//作为一个基类
struct ConnectManager{
    ServerType type;
    int listen_fd{}; //监听的服务器fd
    bool is_connected{};
    std::unordered_map<int, std::shared_ptr<ClientManager>> fd2client;

    int sock_fd{}, epoll_fd{}, server_port{};

    ConnectManager();

    void startServer(); //启动服务器

    void startSocket();

    void startEpoll();

    void listenServer(const std::string& ip, int port); //监听服务器

    void add_client(int fd); //新建一个客户端信息

    void handle_accept(); //接受新的客户端连接

    void handle_read(int fd); //接受客户端的数据

    virtual void close_client(int fd); //关闭客户端连接

    std::shared_ptr<ClientManager> get_client(int fd);

    virtual void add_broadcast(const std::shared_ptr<MessageInfo>& info); //添加广播事件

    void broadcast(); //广播

    static void SetNONBLOCK(int fd);// 设置为非阻塞

    virtual void analyze_package(char * msg, MessageType msg_type, int len, std::shared_ptr<ClientManager>& cm); // 解析proto包

    void close_all_clients();
};
#endif //UNITY_SERVER_CONNECT_H
