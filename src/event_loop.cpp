#include "event_loop.h"
#include "client.h"
#include "utils.h"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>


EventLoop::EventLoop()
    : listen_fd_(-1) , store_(3) {
    epfd_ = epoll_create1(0);
    if (epfd_ < 0) {
        perror("epoll_create1");
        exit(1);
    }
}

EventLoop::~EventLoop() {
    close(epfd_);
}

void EventLoop::set_listen_fd(int fd) {
    listen_fd_ = fd;
    add_epoll(fd, EPOLLIN);
}

void EventLoop::add_epoll(int fd, uint32_t events) {
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    if(epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev)<0) {
        perror("epoll_ctl ADD failed");
        exit(1);
    }
}

void EventLoop::mod_epoll(int fd, uint32_t events) {
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    if(epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev)<0) {
        perror("epoll_ctl MOD failed");
        exit(1);
    }
}

void EventLoop::del_epoll(int fd) {
    if(epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr)<0) {
        perror("epoll_ctl DEL failed");
        exit(1);
    }
}

void EventLoop::run() {
    constexpr int MAX_EVENTS = 64;
    epoll_event events[MAX_EVENTS];

    while (true) {
        int n = epoll_wait(epfd_, events, MAX_EVENTS, -1);
        if (n < 0) {
            perror("epoll_wait");
            continue;
        }

        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;

            // 1️⃣ New incoming connection
            if (fd == listen_fd_) {
                while (true) {
                    int client_fd = accept(listen_fd_, nullptr, nullptr);
                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        // because we are in edge-triggered mode, we must break the loop when there are no more connections to accept
                        perror("accept");
                        break;
                    }

                    utils::set_nonblocking(client_fd);
                    add_epoll(client_fd, EPOLLIN);

                    clients_[client_fd] = new Client(client_fd);
                }
                continue;
            }

            // 2️⃣ Client socket
            Client* client = clients_[fd];

            // Read event
            if (events[i].events & EPOLLIN) {
                if (!client->handle_read(store_)) {
                    del_epoll(fd);
                    delete client;
                    clients_.erase(fd);
                    continue;
                }
            }

            // Write event
            if (events[i].events & EPOLLOUT) {
                client->handle_write();
            }

            // Update epoll interest
            uint32_t ev = EPOLLIN;
            if (client->has_pending_write()) ev |= EPOLLOUT;
            mod_epoll(fd, ev);

        }

        // cleanup expired keys
        store_.active_ttl(); // optional
    }
}
