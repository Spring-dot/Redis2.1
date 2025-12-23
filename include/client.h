#pragma once
#include <string>
#include <cstddef>
#include "datastore.h"

class Client {
public:
    explicit Client(int fd);
    ~Client();

    bool handle_read(DataStore& store);   // returns false if client disconnected
    void handle_write();
    bool has_pending_write() const;

private:
    int fd_;
    std::string inbuf_;
    std::string outbuf_;

    // protocol state
    enum class State {
        READ_COMMAND,
        READ_VALUE
    };

    State state_ = State::READ_COMMAND;
    std::string cur_key_;
    size_t expected_value_len_ = 0;
};
