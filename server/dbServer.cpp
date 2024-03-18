#include <csetjmp>
#include <csignal>
#include "connect/dbConnect.h"

static jmp_buf jmpbuf;
const int TIMEOUT = 5;

void sigint_handler(int signo) {
    std::cout << "The Server receive Ctrl+C, will been closed\n";
    longjmp(jmpbuf, 1);
}

auto dbServer = std::make_unique<DBConnectManager>();

void run() {
    struct epoll_event events[MAX_EVENTS_LIMIT];
    memset(events, 0, sizeof(events));
    while (true) {
        //响应ctrl + c
        if (setjmp(jmpbuf)) {
            std::cout << "Break from Server Listen Loop\n";
            break;
        }

        //每次循环调用依次epoll_wait侦听
        int cnt = epoll_wait(dbServer->epoll_fd, events, MAX_EVENTS_LIMIT, TIMEOUT);
        dbServer->broadcast();

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
                dbServer->close_client(fd);
                continue;
            } else if (event.events & EPOLLHUP) {
                fprintf(stderr, "Epoll hup.\n");
                dbServer->close_client(fd);
                continue;
            }

            if (fd == dbServer->sock_fd) {
                //accept所有client连接请求
                dbServer->handle_accept();
            } else if (event.events & EPOLLIN) {
                //读取数据
                int ret = dbServer->handle_read(fd);
                if(ret == 0){
                    dbServer->close_client(fd);
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigint_handler);

    dbServer->startServer();
    dbServer->db.init_link();
//    dbServer->db.init_data();

    run();

    // Clear
    std::cout << " Try to close all client-connection.\n";
    int ret = shutdown(dbServer->sock_fd, SHUT_WR);
    if (ret < 0) {
        perror("Shutdown error.");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server shuts down." << std::endl;

    return 0;
}