#include "wal.h"
#include "datastore.h"
#include <fstream>
#include <sstream>

WAL::WAL(const std::string& path) : path_(path) {
    worker_ = std::thread(&WAL::writer_loop, this);
}

WAL::~WAL() {
    running_ = false;
    cv_.notify_all();
    worker_.join();
}

void WAL::log(const std::string& record) {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(record);
    }
    cv_.notify_one();
}

void WAL::writer_loop() {
    std::ofstream out(path_, std::ios::app);
    while (running_ || !queue_.empty()) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [&]{ return !queue_.empty() || !running_; });

        while (!queue_.empty()) {
            out << queue_.front() << "\n";
            queue_.pop();
        }
        out.flush();
    }
}

void WAL::replay(DataStore& store) {
    std::ifstream in(path_);
    std::string line;

    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "SET") {
            std::string key, hex;
            size_t len;
            iss >> key >> len >> hex;

            std::string value;
            for (size_t i = 0; i < hex.size(); i += 2) {
                value.push_back(
                    static_cast<char>(std::stoi(hex.substr(i, 2), nullptr, 16))
                );
            }
            store.set(key, value);
        }
        else if (cmd == "EXPIRE") {
            std::string key;
            int sec;
            iss >> key >> sec;
            store.expire(key, sec);
        }
    }
}
