#include "event_loop.h"
#include "client.h"
#include <sys/epoll.h>
#include <unistd.h>

EventLoop::EventLoop() {
    epfd_ = epoll_create1(0);
}

EventLoop::~EventLoop() {
    close(epfd_);
}

void EventLoop::add_fd(int fd) {
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
}

void EventLoop::run() {
    epoll_event events[64];
    while (true) {
        int n = epoll_wait(epfd_, events, 64, -1);
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            // accept / read / write dispatch goes here
        }
    }
}
