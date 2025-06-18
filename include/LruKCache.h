#pragma once
#include "LruCache.h"

template<typename Key, typename Value>
class LruKCache : public LruCache<Key, Value> {
public:
    LruKCache(int capacity, int hisCapacity, int k_):
        LruCache<Key, Value>(capacity),
        k(k_),
        historyPtr(std::make_unique<LruCache<Key, size_t>>(hisCapacity))
    {}
    Value get(Key key){
        Value value{};
        this->get(key, value);
        return value;
    }
    bool get(Key key, Value& value){
        bool inMainCaChe = LruCache<Key, Value>::get(key, value);
        if (inMainCaChe) return true;
        size_t historyCount = historyPtr->get(key);
        historyCount++;
        historyPtr->put(key, historyCount);
        if (historyCount >= k){
            auto it = historytMap.find(key);
            if (it != historytMap.end()){
                value = it->second;
                LruCache<Key, Value>::put(key, value);  // 晋升到主缓存
                historyPtr->remove(key);
                historytMap.erase(it);
                return true;
            }
        }
        return inMainCaChe;
        
    }
    void put(Key key, const Value& value){
        Value existingValue{};
        bool inMainCaChe = LruCache<Key, Value>::get(key, existingValue);
        if (inMainCaChe){
            LruCache<Key, Value>::put(key, value);
            return;
        }
        size_t historyCount = historyPtr->get(key);
        historyCount++;
        historyPtr->put(key, historyCount);
        historytMap[key] = value;
        if (historyCount >= k){
            LruCache<Key, Value>::put(key, value);
            historyPtr->remove(key);
            historytMap.erase(key);
        }
    }
private:
    int k;
    std::unique_ptr<LruCache<Key, size_t>> historyPtr;
    std::unordered_map<Key, Value> historytMap;
};