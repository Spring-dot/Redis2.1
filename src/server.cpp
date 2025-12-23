#include "server.h"
#include "event_loop.h"
#include "utils.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

Server::Server(int port)
    : port_(port), listen_fd_(-1) {}

void Server::setup_socket() {
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Allow quick restart
    int opt = 1;
    if(setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))<0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(listen_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd_);
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd_, SOMAXCONN) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    utils::set_nonblocking(listen_fd_);

    std::cout << "Listening on port " << port_ << std::endl;
}

void Server::run() {
    setup_socket();

    EventLoop loop;
    loop.set_listen_fd(listen_fd_);
    loop.run();
}
