#include "ttl.h"
#include <random>

void TTL::set(const std::string& key, int seconds) {
    expiry_[key] = std::chrono::steady_clock::now()
                 + std::chrono::seconds(seconds);
}

bool TTL::expired(const std::string& key) const {
    auto it = expiry_.find(key);
    if (it == expiry_.end()) return false;
    return std::chrono::steady_clock::now() >= it->second;
}

void TTL::erase(const std::string& key) {
    expiry_.erase(key);
}

void TTL::sample_cleanup(int max_checks) {
    if (expiry_.empty()) return;

    static std::mt19937 rng{std::random_device{}()};
    for (int i = 0; i < max_checks && !expiry_.empty(); ++i) {
        auto it = expiry_.begin();
        std::advance(it, rng() % expiry_.size());
        if (std::chrono::steady_clock::now() >= it->second) {
            expiry_.erase(it);
        }
    }
}
