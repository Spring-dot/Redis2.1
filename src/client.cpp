#include "client.h"
#include <unistd.h>
#include <sstream>

Client::Client(int fd) : fd_(fd) {}
Client::~Client() {
    close(fd_);
}

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

    while (true) {
        // Step 1: read command line
        if (state_ == State::READ_COMMAND) {
            auto pos = inbuf_.find('\n');
            if (pos == std::string::npos)
                break;

            std::string line = inbuf_.substr(0, pos);
            inbuf_.erase(0, pos + 1);

            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;

            if (cmd == "SET") {
                iss >> cur_key_ >> expected_value_len_;
                state_ = State::READ_VALUE;
            }
            else if (cmd == "GET") {
                std::string key, value;
                iss >> key;

                if (store.get(key, value)) {
                    outbuf_ += std::to_string(value.size()) + "\n";
                    outbuf_.append(value);
                } else {
                    outbuf_ += "(nil)\n";
                }
            }
            else if (cmd == "EXPIRE") {
                std::string key;
                int seconds;
                iss >> key >> seconds;

                if (store.exists(key)) {
                    store.expire(key, seconds);
                    outbuf_ += "1\n";
                } else {
                    outbuf_ += "0\n";
                }
            }
            else {
                outbuf_ += "ERR unknown command\n";
            }
        }

        // Step 2: read raw value bytes
        if (state_ == State::READ_VALUE) {
            if (inbuf_.size() < expected_value_len_)
                break;

            std::string value = inbuf_.substr(0, expected_value_len_);
            inbuf_.erase(0, expected_value_len_);

            store.set(cur_key_, value);
            outbuf_ += "OK\n";

            // reset state
            state_ = State::READ_COMMAND;
            cur_key_.clear();
            expected_value_len_ = 0;
        }
    }

    return true;
}

void Client::handle_write() {
    while (!outbuf_.empty()) {
        ssize_t n = write(fd_, outbuf_.data(), outbuf_.size());
        if (n > 0) {
            outbuf_.erase(0, n);
        } else {
            break;
        }
    }
}

bool Client::has_pending_write() const {
    return !outbuf_.empty();
}
