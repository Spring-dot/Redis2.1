#pragma once
#include <unordered_map>

class Client;

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    void add_fd(int fd);
    void remove_fd(int fd);
    void run();

private:
    int epfd_;
    std::unordered_map<int, Client*> clients_;
};
