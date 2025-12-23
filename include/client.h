#pragma once
#include <string>

class Client {
public:
    explicit Client(int fd);
    ~Client();

    bool handle_read();   // returns false if client disconnected
    void handle_write();
    bool has_pending_write() const;

private:
    int fd_;
    std::string inbuf_;
    std::string outbuf_;
};
