#pragma once
#include <string>
#include <unordered_map>
#include "lru.h"

class DataStore {
public:
    explicit DataStore(size_t capacity);

    bool get(const std::string& key, std::string& value);
    void set(const std::string& key, const std::string& value);

private:
    std::unordered_map<std::string, std::string> store_;
    LRUCache lru_;
};
