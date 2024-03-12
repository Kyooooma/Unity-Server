#ifndef UNITY_SERVER_CONNECT_H
#define UNITY_SERVER_CONNECT_H

#include "../common/config.h"
#include "unordered_map"
#include "../client/client.h"
#include "vector"

//作为一个基类
struct ConnectManager{
    ServerType type;
    //游戏开始的所有帧
    std::vector<std::shared_ptr<MessageInfo>> frames;
    int listen_fd{}; //监听的服务器fd
    std::unordered_map<int, std::shared_ptr<ClientManager>> fd2client;

    int sock_fd{}, epoll_fd{};

    ConnectManager();

    void sync(int fd); //追帧 tbd:: 修改为追帧info

    void add_frame(const std::shared_ptr<MessageInfo>& info);

    void startServer(int port); //启动服务器

    void listenServer(const std::string& ip, int port); //监听服务器

    void add_client(int fd); //新建一个客户端信息

    void handle_accept(); //接受新的客户端连接

    void handle_read(int fd); //接受客户端的数据

    void close_client(int fd); //关闭客户端连接

    std::shared_ptr<ClientManager> get_client(int fd);

    void add_broadcast(const std::shared_ptr<MessageInfo>& info); //添加广播事件

    void broadcast(); //广播

    static void SetNONBLOCK(int fd);// 设置为非阻塞

    void analyze_package(char * msg, MessageType msg_type, int len); // 解析proto包
};
#endif //UNITY_SERVER_CONNECT_H
