#include "datastore.h"

DataStore::DataStore(size_t capacity)
    : lru_(capacity) {}

bool DataStore::get(const std::string& key, std::string& value) {
    auto it = store_.find(key);
    if (it == store_.end())
        return false;

    value = it->second;
    lru_.touch(key);
    return true;
}

void DataStore::set(const std::string& key, const std::string& value) {
    bool exists = store_.count(key);

    store_[key] = value;

    if (exists) {
        lru_.touch(key);
    } else {
        lru_.insert(key);
        if (lru_.full()) {
            std::string victim = lru_.evict();
            store_.erase(victim);
        }
    }
}
