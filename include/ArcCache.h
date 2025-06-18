#pragma once

#include "caChePolicy.h"
#include "ArcLfuPart.h"
#include "ArcLruPart.h"
#include <memory>

template<typename Key, typename Value>
class ArcCahce : public caChepolicy<Key, Value>{
public:
    explicit ArcCahce(size_t capacity, size_t transformThreshold)
        : _capacity(capacity)
        , _transformThreshold(transformThreshold)
        , _lruPart(std::make_unique<ArcLruPart<Key, Value>>(_capacity, _transformThreshold))
        , _lfuPart(std::make_unique<ArcLfuPart<Key, Value>>(_capacity, _transformThreshold))
    {}
    ~ArcCahce() override = default;

    void put(Key key, const Value& value) override{
        checkGhostCaches(key);
        // 检查 LFU 部分是否存在该键
        bool inLfu = _lfuPart->contain(key);
        // 更新 LRU 部分缓存
        _lruPart->put(key, value);
        // 如果 LFU 部分存在该键，则更新 LFU 部分
        if (inLfu) 
        {
            _lfuPart->put(key, value);
        }       
    }

    bool get(Key key, Value& value) override 
    {
        checkGhostCaches(key);
        bool shouldTransform = false;
        if (_lruPart->get(key, value, shouldTransform)) 
        {
            if (shouldTransform) 
            {
                _lfuPart->put(key, value);
            }
            return true;
        }
        return _lfuPart->get(key, value);
    }
    
    Value get(Key key) override {
        Value value{};
        this->get(key, value);
        return value;
    }

private:
    bool checkGhostCaches(Key key) 
    {
        bool inGhost = false;
        if (_lruPart->checkGhost(key)) 
        {
            if (_lfuPart->decreaseCapacity()) 
            {
                _lruPart->increaseCapacity();
            }
            inGhost = true;
        } 
        else if (_lfuPart->checkGhost(key)) 
        {
            if (_lruPart->decreaseCapacity()) 
            {
                _lfuPart->increaseCapacity();
            }
            inGhost = true;
        }
        return inGhost;
    }

private:
    size_t _capacity;
    size_t _transformThreshold;
    std::unique_ptr<ArcLruPart<Key, Value>> _lruPart;
    std::unique_ptr<ArcLfuPart<Key, Value>> _lfuPart;
};