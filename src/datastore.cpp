#include "datastore.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ostream>

using namespace std::chrono;

// DataStore::DataStore(size_t max_keys)
//     : max_keys_(max_keys) {}


void DataStore::attach_wal(WAL* wal) {
    wal_ = wal;
}
DataStore::DataStore(size_t capacity)
    : lru_(capacity) {}

bool DataStore::exists(const std::string& key) {
    return store_.count(key);
}

void DataStore::del(const std::string& key) {
    store_.erase(key);
    lru_.erase(key);
    ttl_.erase(key);
}

bool DataStore::get(const std::string& key, std::string& value) {
    if (ttl_.expired(key)) {
        del(key);
        return false;
    }

    auto it = store_.find(key);
    if (it == store_.end()) return false;

    value = it->second;
    lru_.touch(key);
    return true;
}


void DataStore::set(const std::string& key, const std::string& value) {
    bool existed = store_.count(key);
    store_[key] = value;

    if (existed) {
        lru_.touch(key);
        ttl_.erase(key);   // ✅ MUST clear old TTL
    } else {
        lru_.insert(key);
        if (lru_.full()) {
            auto victim = lru_.evict();
            del(victim);
        }
    }

    if (wal_) {
    std::ostringstream oss;
    oss << "SET " << key << " " << value.size() << " ";
    for (unsigned char c : value)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    wal_->log(oss.str());
    }

}


void DataStore::expire(const std::string& key, int seconds) {
    if (store_.count(key)) {
        ttl_.set(key, seconds);
    }

    if (wal_) {
    wal_->log("EXPIRE " + key + " " + std::to_string(seconds));
    }

}

void DataStore::active_ttl() {
    ttl_.sample_cleanup(5); // cheap
}

