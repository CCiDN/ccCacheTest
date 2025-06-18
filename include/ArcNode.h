#pragma once

#include <memory>

template<typename Key, typename Value>
class ArcNode{
public:
    ArcNode(): _accessCount(1), _next(nullptr) {}
    ArcNode(Key key, Value value)
        : _key(key)
        , _value(value)
        , _accessCount(1)
        , _next(nullptr)
    {}
    Key getKey() const {return _key;}
    Value getValue() const {return _value;}
    size_t getAccessCount() const {return _accessCount;}
    void setValue(const Value& value) {_value = value;}
    void incrementAccessCount() {++_accessCount;}


    template<typename K, typename V> friend class ArcLruPart;
    template<typename K, typename V>  friend class ArcLfuPart;
private:
    Key _key;
    Value _value;
    size_t _accessCount;
    std::weak_ptr<ArcNode> _prev;
    std::shared_ptr<ArcNode> _next;
};