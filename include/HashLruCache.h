#pragma once

#include <LruCache.h>
#include <vector>
#include <thread>
#include <cmath>

template<typename Key, typename Value>
class HashLruCache : public caChepolicy<Key, Value>{
public:
    HashLruCache(size_t totalCapacity_, int sliceNum_)
    : totalCapacity(totalCapacity_)
    , sliceNum(sliceNum_ > 0 ? sliceNum_ : std::thread::hardware_concurrency())
    {
        size_t sliceSize = std::ceil(totalCapacity / static_cast<double>(sliceNum));
        for (int i = 0; i < sliceNum; ++i){
            slicePtr.emplace_back(std::make_unique<LruCache<Key, Value>>(sliceSize));
        }
    }
    bool get(Key key, Value& value) override {
        size_t index = Hash(key) % sliceNum;
        return slicePtr[index]->get(key, value);
    }
    Value get(Key key) override {
        Value value{};
        this->get(key, value);
        return value;
    }
    void put(Key key, const Value& value) override {
        size_t index = Hash(key) % sliceNum;
        slicePtr[index]->put(key, value);
    }
    size_t Hash(Key key){
        std::hash<Key> myHash;
        return myHash(key);
    }
private:
    size_t totalCapacity;
    int sliceNum;
    std::vector<std::unique_ptr<LruCache<Key, Value>>> slicePtr;
};