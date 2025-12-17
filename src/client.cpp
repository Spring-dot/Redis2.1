#include "client.h"
#include <unistd.h>

Client::Client(int fd) : fd_(fd) {}

Client::~Client() {
    close(fd_);
}

int Client::fd() const { return fd_; }

void Client::handle_read() {}
void Client::handle_write() {}
bool Client::has_pending_write() const { return !outbuf_.empty(); }
