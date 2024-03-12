#include "connect.h"

ConnectManager::ConnectManager() {
    listen_fd = -1;
    type = DEFAULT;
}

void ConnectManager::SetNONBLOCK(int fd) {
    int flag = fcntl(fd, F_GETFL);
    if (flag < 0) {
        perror("Get flag(fcntl(F_GETFL)) error.");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) < 0) {
        perror("SetNONBLOCK error.");
        exit(EXIT_FAILURE);
    }
}

void ConnectManager::startServer(int port) {
//创建socket
    sock_fd = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sock_fd < 0) {
        perror("Create socket error.");
        exit(EXIT_FAILURE);
    }
    int val = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int)) < 0) {
        perror("Setsockopt error.");
        exit(EXIT_FAILURE);
    }
    //bind
    struct sockaddr_in bind_addr{};
    bind_addr.sin_family = AF_INET;
    //接受任意地址
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_addr.sin_port = htons(port);

    if (bind(sock_fd, (struct sockaddr *) &bind_addr, sizeof(bind_addr)) < 0) {
        perror("Bind error.");
        exit(EXIT_FAILURE);
    }

    //listen
    if (listen(sock_fd, MAX_CONN_LIMIT) < 0) {
        perror("Listen error.");
        exit(EXIT_FAILURE);
    }

    //创建epoll
    epoll_fd = epoll_create(MAX_CONN_LIMIT);
    if (epoll_fd < 0) {
        perror("Create epoll error.");
        exit(EXIT_FAILURE);
    }

    //添加socket进入epoll
    struct epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = sock_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) < 0) {
        perror("Epoll_ctl error.");
        exit(EXIT_FAILURE);
    }
}

void ConnectManager::handle_accept() {
    struct sockaddr_in client_addr{};
    char client_ip_str[INET_ADDRSTRLEN];
    struct epoll_event ev{};
    while (true) {
        socklen_t client_addr_len = sizeof(client_addr);
        int conn_sock_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (conn_sock_fd < 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                //non-blocking模式下无新connection请求
                break;
            } else {
                perror("accept error.");
                exit(EXIT_FAILURE);
            }
        }
        if (!inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip_str, sizeof(client_ip_str))) {
            perror("inet_ntop error.");
            exit(EXIT_FAILURE);
        }
        std::cout << "Accept a client from: " << client_ip_str << ", fd = " << conn_sock_fd << ".\n";
        //设置为non-blocking
        SetNONBLOCK(conn_sock_fd);
        //添加到epoll的侦听中
        ev.events = EPOLLIN;
        ev.data.fd = conn_sock_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock_fd, &ev) < 0) {
            perror("epoll_ctl(EPOLL_CTL_ADD) error.");
            exit(EXIT_FAILURE);
        }
        add_client(conn_sock_fd);
    }
}

void ConnectManager::handle_read(int fd) {
    auto cm = get_client(fd);
    if (cm == nullptr) return;
    while (true) {
        int recv_size = cm->read_data();

        if (recv_size == 0) {
            //关闭连接
            close_client(fd);
            printf("close sock_fd=%d done.\n", fd);
            return;
        }

        if (recv_size < 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                //说明读完了
                return;
            } else {
                perror("read error.");
                return;
            }
        }
        std::cout << "Received from client fd = " << fd << " , received Bytes = " << recv_size << ".\n";
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
            analyze_package(msg, (MessageType) package_type, len);
        }
    }
}

void ConnectManager::close_client(int fd) {
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        perror("EPOLL_CTL_DEL error.");
        return;
    }
    if (close(fd) < 0) {
        perror("Close client error.");
        return;
    }
    std::cout << "Close client fd = " << fd << "\n";
    if (!fd2client.count(fd)) {
        fprintf(stderr, "Del client error: the client fd = %d has already deleted.\n", fd);
        return;
    }
    fd2client.erase(fd);
}

std::shared_ptr<ClientManager> ConnectManager::get_client(int fd) {
    if (!fd2client.count(fd)) {
        fprintf(stderr, "Get client error: the client fd = %d has already deleted.\n", fd);
        return nullptr;
    }
    return fd2client[fd];
}

void ConnectManager::add_broadcast(const std::shared_ptr<MessageInfo> &info) {
    //tbd
    add_frame(info);
    for (auto &[fd, cm]: fd2client) {
        cm->add_info(info);
    }
}

void ConnectManager::broadcast() {
    for (auto &[fd, cm]: fd2client) {
        cm->send_all_message();
    }
}

void ConnectManager::listenServer(const std::string &ip, int port) {
    listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listen_fd < 0) {
        perror("Error creating socket");
        return;
    }
    add_client(listen_fd);//添加监听服务器
    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    serv_addr.sin_port = htons(port);
    if (connect(listen_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        if (errno == EINPROGRESS) {
            // 连接正在进行中，稍后使用 epoll 等待连接完成
        } else {
            perror("Connection failed");
            close_client(listen_fd);
            return;
        }
    } else {
        //说明connect成功
        //将监听服务器添加入epoll中
        struct epoll_event event{};
        event.events = EPOLLIN;  // 监听读事件
        event.data.fd = listen_fd;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) == -1) {
            perror("epoll_ctl: clientSocket");
            close_client(listen_fd);
        }
    }
}

void ConnectManager::add_client(int fd) {
    if (fd2client.count(fd)) {
        fprintf(stderr, "Add client error: the client fd = %d has already added.\n", fd);
        return;
    }
    fd2client[fd] = std::make_shared<ClientManager>(fd);
    if (type == ServerType::GameServer) {
        sync(fd);
    }
}

void ConnectManager::analyze_package(char *msg, MessageType msg_type, int len) {
    if (msg_type == MessageType::MoveInfo) {
        messagek::MoveInfo info;
        if (MessageUtils::deserialize(info, msg, len) < 0) {
            std::cout << "Failed to serialize message." << std::endl;
            return;
        }
        std::cout << "uid:: " << info.uid() << " horizontal:: " << info.horizontal() << " vertical:: "
                  << info.vertical() << " is_atk:: " << info.is_atk() << "\n";
        //需要delete
        auto data = MessageUtils::serialize(info, msg_type, &len);
        auto moveInfo = std::make_shared<MessageInfo>(data, len);
        delete[] data;
        //添加广播
        add_broadcast(moveInfo);
    } else if (msg_type == MessageType::LogInfo) {
        messagek::LogInfo info;
        if (MessageUtils::deserialize(info, msg, len) < 0) {
            std::cout << "Failed to serialize message." << std::endl;
            return;
        }
        std::cout << "username:: " << info.username() << " password:: " << info.password() << "\n";
        info.set_password("SUCCESS");
        //需要delete
        auto data = MessageUtils::serialize(info, msg_type, &len);
        auto logInfo = std::make_shared<MessageInfo>(data, len);
        delete[] data;
        //添加广播
        add_broadcast(logInfo);
    }
}

void ConnectManager::sync(int fd) {
    auto cm = get_client(fd);
    for (const auto &msg: frames) {
        int ret = cm->send_data(msg);
        if (ret < 0) {
            perror("Handle write error.");
            continue;
        } else if (ret == 0) {
            //关闭连接
            close_client(fd);
            printf("close sock_fd=%d done.\n", fd);
            return;
        }
    }
}

void ConnectManager::add_frame(const std::shared_ptr<MessageInfo> &info) {
    if (type == ServerType::GameServer) {
        frames.push_back(info);
    }
}
