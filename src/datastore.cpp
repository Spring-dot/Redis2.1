#include "datastore.h"

DataStore::DataStore(size_t max_bytes) : max_bytes_(max_bytes) {}

bool DataStore::get(const std::string& key, std::string& value) {
    auto it = store_.find(key);
    if (it == store_.end()) return false;
    value = it->second;
    return true;
}

void DataStore::set(const std::string& key, const std::string& value) {
    store_[key] = value;
}

void DataStore::del(const std::string& key) {
    store_.erase(key);
}
