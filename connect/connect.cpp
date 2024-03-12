#include "connect.h"

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
    sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Create socket error.");
        exit(EXIT_FAILURE);
    }
    int val = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int)) < 0) {
        perror("Setsockopt error.");
        exit(EXIT_FAILURE);
    }
    //设置为非阻塞
    SetNONBLOCK(sock_fd);
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
        //同步之前的所有帧
        sync(conn_sock_fd);
    }
}

void ConnectManager::add_client(int fd) {
    if (fd2client.count(fd)) {
        fprintf(stderr, "Add client error: the client fd = %d has already added.\n", fd);
        return;
    }
    fd2client[fd] = std::make_shared<ClientManager>();
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

void ConnectManager::handle_read(int fd) {
    auto cm = get_client(fd);
    if (cm == nullptr) return;
    while (true) {
        int recv_size = messageManager->read_data(fd);

        if (recv_size == 0) {
            //关闭连接
            close_client(fd);
            printf("close sock_fd=%d done.\n", fd);
            return;
        }

        if (recv_size < 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                return;
            } else {
                perror("read error.");
                return;
            }
        }
        std::cout << "Received from client fd = " << fd << " , received Bytes = " << recv_size << ".\n";

        messageManager->process_data(cm);
    }
}

std::shared_ptr<ClientManager> ConnectManager::get_client(int fd) {
    if (!fd2client.count(fd)) {
        fprintf(stderr, "Get client error: the client fd = %d has already deleted.\n", fd);
        return nullptr;
    }
    return fd2client[fd];
}

ConnectManager::ConnectManager() {
    messageManager = std::make_unique<MessageManager>();
}

void ConnectManager::broadcast(int curFrame) {
    messagek::SequenceNotice frameSequence;
    frameSequence.set_sequence(curFrame);
    int tot_len;
    //new
    auto frameInfo = ClientManager::serialize(frameSequence, MessageType::SequenceNotice, &tot_len);
    std::shared_ptr<MessageInfo> info = nullptr;
    if(frameInfo != nullptr){
        info = std::make_shared<MessageInfo>(frameInfo, tot_len);
        //delete
        delete[] frameInfo;
    }else{
        std::cout << "send frameSequence error.";
    }
    for(auto &[fd, cm] : fd2client){
        if(info != nullptr){
            cm->dq.push_front(info);
        }
        while(!cm->dq.empty()){
            auto msg = cm->dq.front();
            cm->dq.pop_front();
            int ret = send(fd, msg->msg, msg->len, 0);
            if(ret < 0){
                perror("Handle write error.");
                continue;
            }else if(ret == 0){
                //关闭连接
                close_client(fd);
                printf("close sock_fd=%d done.\n", sock_fd);
                break;
            }
        }
    }
}

void ConnectManager::sync(int fd) {
    for(const auto& msg : frames){
        int ret = send(fd, msg->msg, msg->len, 0);
        if(ret < 0){
            perror("Handle write error.");
            continue;
        }else if(ret == 0){
            //关闭连接
            close_client(fd);
            printf("close sock_fd=%d done.\n", fd);
            return;
        }
    }
}

void ConnectManager::add_broadcast(char *val, int length) {
    std::cout << "add_broadcast" << "\n";
    auto msg_info = std::make_shared<MessageInfo>(val, length);
    for(auto &[fd, cm] : fd2client){
        cm->add_info(msg_info);
    }
    frames.push_back(msg_info);
}
