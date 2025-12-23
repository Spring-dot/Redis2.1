#pragma once

#include "datastore.h"
#include <cstdint>
#include <unordered_map>

class Client;

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    void set_listen_fd(int fd);
    void run();

private:
    int epfd_;
    int listen_fd_;
    std::unordered_map<int, Client*> clients_;
    DataStore store_;

    void add_epoll(int fd, uint32_t events);
    void mod_epoll(int fd, uint32_t events);
    void del_epoll(int fd);
};
