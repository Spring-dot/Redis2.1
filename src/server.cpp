#include "server.h"
#include "event_loop.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

Server::Server(int port) : port_(port), listen_fd_(-1) {}

void Server::setup_socket() {
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
}

void Server::run() {
    setup_socket();
    EventLoop loop;
    loop.add_fd(listen_fd_);
    loop.run();
}
