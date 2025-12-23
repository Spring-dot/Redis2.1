#include "client.h"
#include <unistd.h>

Client::Client(int fd)
    : fd_(fd) {}

Client::~Client() {
    close(fd_);
}

// Read data from socket
bool Client::handle_read() {
    char buf[4096];

    while (true) {
        ssize_t n = read(fd_, buf, sizeof(buf));
        if (n > 0) {
            inbuf_.append(buf, n);
            outbuf_.append(buf, n);  // echo behavior
        } else if (n == 0) {
            // client closed connection
            return false;
        } else {
            // EAGAIN or EWOULDBLOCK
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            return false;
        }
    }
    return true;
}

// Write pending data to socket
void Client::handle_write() {
    while (!outbuf_.empty()) {
        ssize_t n = write(fd_, outbuf_.data(), outbuf_.size());
        if (n > 0) {
            outbuf_.erase(0, n);
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            // On error, we simply stop writing for now
            return;
        }
    }
}

bool Client::has_pending_write() const {
    return !outbuf_.empty();
}
