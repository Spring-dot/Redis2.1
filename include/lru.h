#pragma once
#include <list>
#include <string>
#include <unordered_map>

class LRUCache {
public:
    explicit LRUCache(size_t capacity);

    void touch(const std::string& key);
    void insert(const std::string& key);
    void erase(const std::string& key);
    bool full() const;
    std::string evict();

private:
    size_t capacity_;
    std::list<std::string> order_; // front = most recent
    std::unordered_map<std::string, std::list<std::string>::iterator> pos_;
};
