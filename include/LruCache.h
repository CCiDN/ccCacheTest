#pragma once

#include <list>
#include <unordered_map>
#include <mutex>
#include <memory>

#include "caChePolicy.h"

template<typename Key, typename Value> class LruCache;

template<typename Key, typename Value> 
class LruNode{
public:
    LruNode(Key key, Value value):
        _key(key),
        _val(value),
        accessCount(1)
    {}
    friend class LruCache<Key, Value>;

    Key getKey() const {return _key;}
    Value getValue() const {return _val;}
    void setValue(const Value& val) {_val = val;}
    size_t getAccessCount() const {return accessCount;}
    void incrementAccessCount() {++accessCount;}

private:
    Key _key;
    Value _val;
    size_t accessCount;
    std::weak_ptr<LruNode<Key, Value>> prev;
    std::shared_ptr<LruNode<Key, Value>> next;
};

template<typename Key, typename Value>
class LruCache : public caChepolicy<Key, Value>{
public:
    using LruNodeType = LruNode<Key, Value>;
    using NodePtr = std::shared_ptr<LruNodeType>;
    using NodeMap = std::unordered_map<Key, NodePtr>;
    LruCache(int capacity_):capacity(capacity_){
        init();
    }
    ~LruCache() override = default;
    void put(Key key, const Value& value) override{
        if (capacity <= 0) return;
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if (it != nodeMap_.end()){
            updateExistingNode(it->second, value);
            return;
        }
        addNode(key, value);
    }
    bool get(Key key, Value& value) override{
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if (it != nodeMap_.end()){
            updateLocating(it->second);
            value = it->second->getValue();
            return true;
        }
        return false;
    }
    Value get(Key key){
        Value value{};
        get(key, value);
        return value;
    }
    void remove(Key key){
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if (it != nodeMap_.end()){
            removeNode(it->second);
            nodeMap_.erase(key);
        }
    }
private:
    void init(){
        dummyHead = std::make_shared<LruNode<Key, Value>>(Key(), Value());
        dummyTail = std::make_shared<LruNode<Key, Value>>(Key(), Value());
        dummyHead->next = dummyTail;
        dummyTail->prev = dummyHead;
    }
    void updateExistingNode(NodePtr& node, const Value& value){
        node->setValue(value);
        updateLocating(node);
    }
    void addNode(const Key& key,const Value& value){
        if (nodeMap_.size() >= capacity) evictLeastRecent();
        NodePtr node = std::make_shared<LruNode<Key, Value>>(key, value);
        nodeMap_[key] = node;
        insertNode(node);
    }
    void updateLocating(NodePtr node){
        removeNode(node);
        insertNode(node);
    }
    void removeNode(NodePtr node){
        if (!node->prev.expired() && node->next){
            auto prev_ = node->prev.lock();
            prev_->next = node->next;
            node->next->prev = prev_;
            node->next = nullptr;
        }
    }
    void insertNode(NodePtr node){
        node->next = dummyTail;
        node->prev = dummyTail->prev;
        dummyTail->prev.lock()->next = node;
        dummyTail->prev = node;
    }
    void evictLeastRecent(){
        NodePtr leastRecentNode = dummyHead->next;
        removeNode(leastRecentNode);
        nodeMap_.erase(leastRecentNode->getKey());
    }
private:
    int capacity;
    NodeMap nodeMap_;
    std::mutex mutex_;
    NodePtr dummyHead;
    NodePtr dummyTail;
};

