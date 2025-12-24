#pragma once
#include <unordered_map>
#include <string>
#include <chrono>

class TTL {
public:
    void set(const std::string& key, int seconds);
    bool expired(const std::string& key) const;
    void erase(const std::string& key);

    // optional active cleanup
    void sample_cleanup(int max_checks);

private:
    using TP = std::chrono::steady_clock::time_point;
    std::unordered_map<std::string, TP> expiry_;
};
