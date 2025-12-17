#pragma once
#include <string>
#include <unordered_map>

class LRUCache;
class TTLManager;

class DataStore {
public:
    DataStore(size_t max_bytes);

    bool get(const std::string& key, std::string& value);
    void set(const std::string& key, const std::string& value);
    void del(const std::string& key);

private:
    std::unordered_map<std::string, std::string> store_;
    LRUCache* lru_;
    TTLManager* ttl_;
    size_t max_bytes_;
};
