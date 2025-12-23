#include "lru.h"

LRUCache::LRUCache(size_t capacity)
    : capacity_(capacity) {}

void LRUCache::touch(const std::string& key) {
    auto it = pos_.find(key);
    if (it == pos_.end()) return;

    order_.erase(it->second);
    order_.push_front(key);
    it->second = order_.begin();
}

void LRUCache::insert(const std::string& key) {
    order_.push_front(key);
    pos_[key] = order_.begin();
}

void LRUCache::erase(const std::string& key) {
    auto it = pos_.find(key);
    if (it == pos_.end()) return;
    order_.erase(it->second);
    pos_.erase(it);
}

bool LRUCache::full() const {
    return pos_.size() > capacity_;
}

std::string LRUCache::evict() {
    std::string key = order_.back();
    order_.pop_back();
    pos_.erase(key);
    return key;
}
