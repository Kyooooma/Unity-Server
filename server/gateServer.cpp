#include <csetjmp>
#include <csignal>
#include "connect/gateConnect.h"

static jmp_buf jmpbuf;
const int TIMEOUT = 5;

void sigint_handler(int signo) {
    std::cout << "The Server receive Ctrl+C, will been closed\n";
    longjmp(jmpbuf, 1);
}

auto gateServer = std::make_unique<GateConnectManager>();

void run() {
    struct epoll_event events[MAX_EVENTS_LIMIT];
    memset(events, 0, sizeof(events));
    while (true) {
        //响应ctrl + c
        if (setjmp(jmpbuf)) {
            std::cout << "Break from Server Listen Loop\n";
            break;
        }
        //检查与gameserver的连接
        if (gateServer->listen_fd < 0) {
            //retry
            gateServer->close_all_clients();
            sleep(1);
            std::cout << "retry link to gameServer....\n";
            gateServer->listenServer("127.0.0.1", DEFAULT_GAMESERVER_PORT);
            if (gateServer->listen_fd == -1) {
                continue;
            }
        }

        //每次循环调用依次epoll_wait侦听
        int cnt = epoll_wait(gateServer->epoll_fd, events, MAX_EVENTS_LIMIT, TIMEOUT);
        gateServer->broadcast();

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
                gateServer->close_client(fd);
                continue;
            } else if (event.events & EPOLLHUP) {
                fprintf(stderr, "Epoll hup.\n");
                gateServer->close_client(fd);
                continue;
            }
            if(!gateServer->is_connected){
                if(fd == gateServer->listen_fd){
                    std::cout << "Connect to gameServer success.\n";
                    gateServer->startSocket();
                    gateServer->is_connected = true;
                }else{
                    continue;
                }
            }
            if (fd == gateServer->sock_fd) {
                //accept所有client连接请求
                gateServer->handle_accept();
            } else if (event.events & EPOLLIN) {
                //读取数据
                gateServer->handle_read(fd);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigint_handler);

    gateServer->startServer();
    gateServer->listenServer("127.0.0.1", DEFAULT_GAMESERVER_PORT);

    run();

    // Clear
    std::cout << " Try to close all client-connection.\n";
    int ret = shutdown(gateServer->sock_fd, SHUT_WR);
    if (ret < 0) {
        perror("Shutdown error.");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server shuts down." << std::endl;

    return 0;
}