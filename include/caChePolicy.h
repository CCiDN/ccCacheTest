#pragma once

template<typename Key, typename Value>
class caChepolicy{
public:
    virtual ~caChepolicy() {};
    virtual void put(Key key, const Value& value) = 0;
    virtual bool get(Key key, Value& value) = 0;
    virtual Value get(Key key) = 0;
};