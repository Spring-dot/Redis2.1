#pragma once
#include <string>
#include <thread>
#include <mutex>
#include <vector>

class WAL {
public:
    explicit WAL(const std::string& path);
    ~WAL();

    void append(const std::string& entry);
    void replay();
    void start_bg_flush();

private:
    std::string path_;
    std::vector<std::string> buffer_;
    std::mutex mtx_;
    std::thread bg_thread_;
    bool running_;
};
