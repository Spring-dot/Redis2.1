#pragma once
#include <string>
#include <unordered_map>
#include "lru.h"
#include "ttl.h"

class DataStore {
public:
    explicit DataStore(size_t capacity);

    bool get(const std::string& key, std::string& value);
    void set(const std::string& key, const std::string& value);
    bool exists(const std::string& key);
    void del(const std::string& key);

    void expire(const std::string& key, int seconds);
    void active_ttl();   // optional

private:
    std::unordered_map<std::string, std::string> store_;
    LRUCache lru_;
    TTL ttl_;
};

