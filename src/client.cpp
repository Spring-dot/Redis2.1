#include "client.h"

#include <unistd.h>
#include <sstream>

Client::Client(int fd)
    : fd_(fd) {}

Client::~Client() {
    close(fd_);
}

// Read data from socket
bool Client::handle_read(DataStore& store) {
    char buf[4096];

    while (true) {
        ssize_t n = read(fd_, buf, sizeof(buf));
        if (n > 0) {
            inbuf_.append(buf, n);
        } else if (n == 0) {
            return false;
        } else {
            break;
        }
    }

    // Process complete lines
    while (true) {
        auto pos = inbuf_.find('\n');
        if (pos == std::string::npos) break;

        std::string line = inbuf_.substr(0, pos);
        inbuf_.erase(0, pos + 1);

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "SET") {
            std::string key, value;
            iss >> key >> value;
            store.set(key, value);
            outbuf_ += "OK\n";
        }
        else if (cmd == "GET") {
            std::string key, value;
            iss >> key;
            if (store.get(key, value))
                outbuf_ += value + "\n";
            else
                outbuf_ += "(nil)\n";
        }
        else {
            outbuf_ += "ERR unknown command\n";
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
