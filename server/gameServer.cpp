#include <csetjmp>
#include <csignal>
#include "connect/gameConnect.h"

static jmp_buf jmpbuf;
int curFrame = 0;

void sigint_handler(int signo) {
    std::cout << "The Server receive Ctrl+C, will been closed\n";
    longjmp(jmpbuf, 1);
}

auto gameServer = std::make_unique<GameConnectManager>();

void run() {
    struct epoll_event events[MAX_EVENTS_LIMIT];
    memset(events, 0, sizeof(events));
    messagek::SequenceNotice frameNotice;
    auto lastExecutionTime = std::chrono::high_resolution_clock::now();
    while (true) {
        //响应ctrl + c
        if (setjmp(jmpbuf)) {
            std::cout << "Break from Server Listen Loop" << std::endl;
            break;
        }
        //检查与db的连接
        if (gameServer->listen_fd < 0) {
            //retry
            std::cout << "retry link to dbServer...." << std::endl;
            gameServer->listenServer("127.0.0.1", DEFAULT_DBSERVER_PORT);
        }

        //每次循环调用依次epoll_wait侦听
        int cnt = epoll_wait(gameServer->epoll_fd, events, MAX_EVENTS_LIMIT, EPOLL_TIMEOUT);

        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                currentTime - lastExecutionTime).count();
        if (elapsedMilliseconds >= EXECUTE_INTERVAL) {
            curFrame += 1;
            frameNotice.set_sequence(curFrame);
            auto frameInfo = MessageUtils::serialize(frameNotice, MessageType::SequenceNotice);
            //广播
            gameServer->add_broadcast(frameInfo);
            gameServer->broadcast();
            gameServer->add_frame(frameInfo);
            // 更新上一次执行的时间点
            lastExecutionTime = currentTime;
        }

        if (cnt < 0) {
            perror("Epoll_wait error.");
            exit(EXIT_FAILURE);
        } else if (cnt == 0) {
            // 说明没有数据了
            continue;
        }

        for (int i = 0; i < cnt; i++) {
            auto event = events[i];
            int fd = event.data.fd;
            if (event.events & EPOLLERR) {
                fprintf(stderr, "Epoll error.\n");
                gameServer->close_client(fd);
                continue;
            } else if (event.events & EPOLLHUP) {
                fprintf(stderr, "Epoll hup.\n");
                gameServer->close_client(fd);
                continue;
            }
            if(!gameServer->is_connected){
                if(fd == gameServer->listen_fd){
                    std::cout << "Connect to dbServer success." << std::endl;
                    gameServer->is_connected = true;
                    continue;
                }
            }
            if (fd == gameServer->sock_fd) {
                //accept所有client连接请求
                gameServer->handle_accept();
            } else if (event.events & EPOLLIN) {
                //读取数据
                gameServer->handle_read(fd);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigint_handler);

    gameServer->startServer();
    gameServer->listenServer("127.0.0.1", DEFAULT_DBSERVER_PORT);

    run();

    // Clear
    std::cout << " Try to close all client-connection.\n";
    int ret = shutdown(gameServer->sock_fd, SHUT_WR);
    if (ret < 0) {
        perror("Shutdown error.");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server shuts down." << std::endl;

    return 0;
}