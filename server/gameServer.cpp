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

void run(){
    struct epoll_event events[MAX_EVENTS_LIMIT];
    memset(events, 0, sizeof(events));
    messagek::SequenceNotice frameNotice;
    int tot_len;
    auto lastExecutionTime = std::chrono::high_resolution_clock::now();
    while (true) {
        //响应ctrl + c
        if (setjmp(jmpbuf)) {
            std::cout << "Break from Server Listen Loop\n";
            break;
        }

        //每次循环调用依次epoll_wait侦听
        int cnt = epoll_wait(gameServer->epoll_fd, events, MAX_EVENTS_LIMIT, EPOLL_TIMEOUT);

        auto currentTime = std::chrono::high_resolution_clock::now();
        // 计算时间间隔
        auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastExecutionTime).count();
        // 如果时间间隔大于等于 EXECUTE_INTERVAL 毫秒，则执行操作
        if (elapsedMilliseconds >= EXECUTE_INTERVAL) {
            curFrame += 1;
            frameNotice.set_sequence(curFrame);
            auto data = MessageUtils::serialize(frameNotice, MessageType::SequenceNotice, &tot_len);
            auto frameInfo = std::make_shared<MessageInfo>(data, tot_len);
            //广播
            gameServer->add_broadcast(frameInfo);
            gameServer->broadcast();
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
    int port;
    if (argc == 2) {
        if ((port = atoi(argv[1])) < 0) {
            fprintf(stderr, "USAGE: %s [port]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    } else if (argc < 2) {
        port = DEFAULT_PORT;
    } else {
        fprintf(stderr, "USAGE: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, sigint_handler);

    gameServer->startServer(port);

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