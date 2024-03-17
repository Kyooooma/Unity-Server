#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <csignal>
#include "client/client.h"
#include "message/messageUtils.h"
#include "proto/msg.pb.h"

// 模拟用户登录请求
void login(int id) {
    std::string username = std::to_string(id);
    // 创建套接字
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Error: Failed to create socket." << std::endl;
        return;
    }

    // 设置服务器地址和端口
    struct sockaddr_in server_address{};
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8766);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 连接到服务器
    if (connect(client_socket, reinterpret_cast<struct sockaddr *>(&server_address), sizeof(server_address)) == -1) {
        std::cerr << "Error: Failed to connect to server." << std::endl;
        close(client_socket);
        return;
    }
    for (int i = 1; i <= 1; i++) {

        auto cm = std::make_shared<ClientManager>(client_socket);
        // 发送消息给服务器
        messagek::LogInfo info;
        info.set_recuser(username);
        info.set_password(username);
        info.set_username(username);
        auto res = MessageUtils::serialize(info, MessageType::LogInfo);
        int ret = cm->send_data(res);
        if (ret == -1) {
            std::cerr << "Error: Failed to send message to server." << std::endl;
            close(client_socket);
            return;
        }

        while (true) {
            int recv_size = cm->read_data();

            if (recv_size == 0) {
                //关闭连接
                close(cm->fd);
                printf("close sock_fd=%d done.\n", cm->fd);
                break;
            }

            if (recv_size < 0) {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                    //说明读完了
                    break;
                } else {
                    perror("read error.");
                    break;
                }
            }
            cm->offset += recv_size;
            //解包
            while (true) {
                int package_type, len;
                auto msg = cm->deserialize(&len, &package_type);
                if (msg == nullptr) {
                    //解包完成
                    cm->calc_data();
                    break;
                }
                if (package_type == MessageType::NoticeInfo) {
                    messagek::NoticeInfo notice;
                    MessageUtils::deserialize(notice, msg, len);
//                    std::cout << "msg:: " << notice.msg() << " code:: " << notice.code() << " recuser:: "
//                              << notice.recuser() << "\n";
                    //接受到自己的登录消息
                    if (notice.msg() == username){
                        return;
                    }
                }
            }
        }
    }

}

int main() {
    // 设置并发用户数
    const int num_users = 1000;
    std::vector<std::thread> threads;
    std::atomic<int> user_id(1);

    auto lastExecutionTime = std::chrono::high_resolution_clock::now();

    // 创建多个线程模拟用户登录
    threads.reserve(num_users);
    for (int i = 0; i < num_users; ++i) {
        threads.emplace_back([&user_id]() {
            int id = user_id++;
            login(id);
        });
    }

    for (auto &thread: threads) {
        thread.join();
    }
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - lastExecutionTime).count();
    std::cout << "cost:: " << elapsedMilliseconds << ".\n";

    return 0;
}