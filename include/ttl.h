#pragma once
#include <string>
#include <unordered_map>
#include <chrono>

class TTLManager {
public:
    void set_expiry(const std::string& key, int seconds);
    bool is_expired(const std::string& key);
    void sample_cleanup();

private:
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> expiry_;
};
