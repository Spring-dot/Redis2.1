#pragma once

class Server {
public:
    explicit Server(int port);
    void run();

private:
    int port_;
    int listen_fd_;
    void setup_socket();
};
