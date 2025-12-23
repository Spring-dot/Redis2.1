#pragma once
#include <string>
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
};
