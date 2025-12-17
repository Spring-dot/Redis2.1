#pragma once
#include <string>

class LRUCache {
public:
    explicit LRUCache(size_t max_bytes);

    void touch(const std::string& key);
    void insert(const std::string& key, size_t size);
    void remove(const std::string& key);

private:
    size_t max_bytes_;
    size_t used_bytes_;
};
