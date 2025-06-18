#pragma once

#include "ArcNode.h"
#include <unordered_map>
#include <map>
#include <mutex>

template<typename Key, typename Value>
class ArcLruPart{
public:
    using NodeType = ArcNode<Key, Value>;
    using NodePtr = std::shared_ptr<NodeType>;
    using NodeMap = std::unordered_map<Key, NodePtr>;
    explicit ArcLruPart(size_t capacity, size_t transformThreshold)
        : _capacity(capacity)
        , _ghostCapacity(capacity)
        , _transformThreshold(transformThreshold)
    {
        initializeLists();
    }

    bool put(Key _key,const Value& _value){
        if (_capacity == 0) return false;
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _mainCache.find(_key);
        if (it != _mainCache.end()) {
            return updateExistingNode(it->second, _value);
        }
        return addNode(_key, _value);
    }

    bool get(Key _key, Value& _value, bool& shouldTransform) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _mainCache.find(_key);
        if (it != _mainCache.end()) {
            _value = it->second->getValue();
            shouldTransform = updateNodeAccess(it->second);
            return true;
        }
        return false;
    }

    bool checkGhost(Key key) {
        auto it = _ghostCache.find(key);
        if (it != _ghostCache.end()) {
            removeFromGhost(it->second);
            _ghostCache.erase(it);
            return true;
        }
        return false;
    }

    void increaseCapacity() {++_capacity;}

    bool decreaseCapacity() {
        if (_capacity <= 0) return false;
        if (_mainCache.size() == _capacity) {
             evictLeastRecent();
        }
        --_capacity;
        return true;
    }

private:
    void initializeLists() 
    {
        _mainHead = std::make_shared<NodeType>();
        _mainTail = std::make_shared<NodeType>();
        _mainHead->_next = _mainTail;
        _mainTail->_prev = _mainHead;

        _ghostHead = std::make_shared<NodeType>();
        _ghostTail = std::make_shared<NodeType>();
        _ghostHead->_next = _ghostTail;
        _ghostTail->_prev = _ghostHead;
    }

    bool updateExistingNode(NodePtr node, const Value& value) {
        node->setValue(value);
        moveToFront(node);
        return true;
    }

    bool addNode(const Key& key, const Value& value) {
        if (_capacity == _mainCache.size()) evictLeastRecent();
        NodePtr tempNode = std::make_shared<NodeType> (key, value);
        _mainCache[key] = tempNode;
        addToFront(tempNode);
        return true;
    }

    bool updateNodeAccess(NodePtr node) {
        node->incrementAccessCount();
        moveToFront(node);
        return node->getAccessCount() >= _transformThreshold;
    }

    void moveToFront(NodePtr node) {
        if (!node->_prev.expired() && node->_next) {
            auto prev = node->_prev.lock();
            prev->_next = node->_next;
            node->_next->_prev = node->_prev;
            node->_next = nullptr;
        }
        addToFront(node);
    }

    void addToFront(NodePtr node) {
        node->_next = _mainHead->_next;
        _mainHead->_next->_prev = node;
        _mainHead->_next = node;
        node->_prev = _mainHead;
    }

    void evictLeastRecent() {
        NodePtr leastRecent = _mainTail->_prev.lock();
        if (!leastRecent || leastRecent == _mainHead) return;
        removeFromMain(leastRecent);
        if (_ghostCache.size() >= _ghostCapacity) removeOldestGhost();
        addToGhost(leastRecent);
        _mainCache.erase(leastRecent->getKey());
    }

     void removeFromMain(NodePtr node) 
    {
        if (!node->_prev.expired() && node->_next) {
            auto prev = node->_prev.lock();
            prev->_next = node->_next;
            node->_next->_prev = node->_prev;
            node->_next = nullptr; // 清空指针，防止悬垂引用
        }
    }

    void removeFromGhost(NodePtr node) 
    {
        if (!node->_prev.expired() && node->_next) {
            auto prev = node->_prev.lock();
            prev->_next = node->_next;
            node->_next->_prev = node->_prev;
            node->_next = nullptr; // 清空指针，防止悬垂引用
        }
    }

    void addToGhost(NodePtr node) 
    {
        node->_next = _ghostTail;
        node->_prev = _ghostTail->_prev;
        if (!_ghostTail->_prev.expired()) {
            _ghostTail->_prev.lock()->_next = node;
        }
        _ghostTail->_prev= node;
        _ghostCache[node->getKey()] = node;
    }
    void removeOldestGhost() 
    {
        NodePtr oldestGhost = _ghostHead->_next;
        if (oldestGhost != _ghostTail) 
        {
            removeFromGhost(oldestGhost);
            _ghostCache.erase(oldestGhost->getKey());
        }
    } 
private:
    size_t _capacity;
    size_t _transformThreshold;
    size_t _ghostCapacity;
    
    std::mutex _mutex;
    NodeMap _mainCache;
    NodeMap _ghostCache;

    NodePtr _mainHead;
    NodePtr _mainTail;

    NodePtr _ghostHead;
    NodePtr _ghostTail;
};