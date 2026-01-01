#pragma once
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

class WAL {
public:
    explicit WAL(const std::string& path);
    ~WAL();

    void log(const std::string& record);
    void replay(class DataStore& store);

private:
    void writer_loop();

    std::string path_;
    std::thread worker_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<std::string> queue_;
    std::atomic<bool> running_{true};
};
