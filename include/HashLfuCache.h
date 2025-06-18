#pragma once

#include "caChePolicy.h"
#include "LfuCache.h"
#include <vector>
#include <climits>

template<typename Key, typename Value>
class HashLfuCache : public caChepolicy<Key, Value>{
public:
    HashLfuCache(size_t capacity, int sliceNum, int maxAverrageNum)
    :_totalCapacity(capacity)
    ,_sliceNum(sliceNum)
    {
        size_t sliceSize = std::ceil(capacity / static_cast<double>(sliceNum));
        for (int i = 0; i < sliceNum; ++i){
            _slicePtr.emplace_back(std::make_unique<LfuCache<Key, Value>>(sliceSize, maxAverrageNum));
        }
    }
    ~HashLfuCache() override = default;
    void put(Key _key,const Value& _value) override {
        int index = Hash(_key) % _sliceNum;
        _slicePtr[index]->put(_key, _value);
    }
    bool get(Key _key, Value& _value) override {
        int index = Hash(_key) % _sliceNum;
        return _slicePtr[index]->get(_key, _value);
    }
    Value get(Key _key) override {
        int index = Hash(_key) % _sliceNum;
        return _slicePtr[index]->get(_key);
    }
    int Hash(Key _key){
        std::hash<Key> myhash;
        return myhash(_key);
    }
private:
    size_t _totalCapacity;
    int _sliceNum;
    std::vector<std::unique_ptr<LfuCache<Key, Value>>> _slicePtr;
};