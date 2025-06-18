#pragma once

#include "ArcNode.h"
#include <unordered_map>
#include <map>
#include <list>
#include <mutex>

template<typename Key, typename Value>
class ArcLfuPart{
public:
    using NodeType = ArcNode<Key, Value>;
    using NodePtr = std::shared_ptr<NodeType>;
    using NodeMap = std::unordered_map<Key, NodePtr>;
    using FreMap = std::map<size_t, std::list<NodePtr>>;

    explicit ArcLfuPart(size_t capacity, size_t transformThreshold)
        : capacity_(capacity)
        , ghostCapacity_(capacity)
        , transformThreshold_(transformThreshold)
        , minFreq_(0)
    {
        initializeLists();
    }

    bool put(Key key, const Value& value) {
        if (capacity_ == 0) return false;
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = mainCache_.find(key);
        if (it != mainCache_.end()) 
        {
            return updateExistingNode(it->second, value);
        }
        return addNewNode(key, value);
    }

    bool get(Key key, Value& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = mainCache_.find(key);
        if (it != mainCache_.end()) 
        {
            updateNodeFrequency(it->second);
            value = it->second->getValue();
            return true;
        }
        return false;       
    }

    bool contain(Key key) {
        return mainCache_.find(key) != mainCache_.end();
    }

    bool checkGhost(Key key) {
        auto it = ghostCache_.find(key);
        if (it != ghostCache_.end()) 
        {
            removeFromGhost(it->second);
            ghostCache_.erase(it);
            return true;
        }
        return false;
    }

    void increaseCapacity() { ++capacity_; }

    bool decreaseCapacity() {
        if (capacity_ <= 0) return false;
        if (mainCache_.size() == capacity_) 
        {
            evictLeastFrequent();
        }
        --capacity_;
        return true;
    }

private:
    void initializeLists() {
        ghostHead_ = std::make_shared<NodeType>();
        ghostTail_ = std::make_shared<NodeType>();
        ghostHead_->_next = ghostTail_;
        ghostTail_->_prev = ghostHead_;
    }

    bool updateExistingNode(NodePtr node, const Value& value) {
        node->setValue(value);
        updateNodeFrequency(node);
        return true;
    }

    bool addNewNode(const Key& key, const Value& value) {
        if (capacity_ == mainCache_.size()) evictLeastFrequent();
        NodePtr newNode = std::make_shared<NodeType>(key, value);
        mainCache_[key] = newNode;
        // 将新节点添加到频率为1的列表中
        if (freMap_.find(1) == freMap_.end()) 
        {
            freMap_[1] = std::list<NodePtr>();
        }
        freMap_[1].push_back(newNode);
        minFreq_ = 1;
        return true;
    }

    void updateNodeFrequency(NodePtr node) {
        size_t oldFreq = node->getAccessCount();
        node->incrementAccessCount();
        size_t newFreq = node->getAccessCount();

        auto& oldList = freMap_[oldFreq];
        oldList.remove(node);

        if (oldList.empty()) 
        {
            freMap_.erase(oldFreq);
            if (oldFreq == minFreq_) 
            {
                minFreq_ = newFreq;
            }
        }

        // 添加到新频率列表
        if (freMap_.find(newFreq) == freMap_.end()) 
        {
            freMap_[newFreq] = std::list<NodePtr>();
        }
        freMap_[newFreq].push_back(node);
    }

    void evictLeastFrequent() {
        if (freMap_.empty()) return;

        auto& minFreqList = freMap_[minFreq_];
        if (minFreqList.empty()) return;
        
        NodePtr leastNode = minFreqList.front();
        minFreqList.pop_front();

        if (minFreqList.empty()) 
        {
            freMap_.erase(minFreq_);
            // 更新最小频率
            if (!freMap_.empty()) 
            {
                minFreq_ = freMap_.begin()->first;
            }
        }    

        // 将节点移到幽灵缓存
        if (ghostCache_.size() == ghostCapacity_) 
        {
            removeOldestGhost();
        }
        addToGhost(leastNode);
        
        // 从主缓存中移除
        mainCache_.erase(leastNode->getKey());
    }

    void removeFromGhost(NodePtr node) {
        if (!node->_prev.expired() && node->_next) {
            auto pre = node->_prev.lock();
            pre->_next = node->_next;
            node->_next->_prev = node->_prev;
            node->_next = nullptr;
        }
    }

    void addToGhost(NodePtr node) {
        node->_next = ghostTail_;
        node->_prev = ghostTail_->_prev;

        if (!ghostTail_->_prev.expired()) {
            ghostTail_->_prev.lock()->_next = node;
        }
        ghostTail_->_prev = node;
        ghostCache_[node->getKey()] = node;
    }

    void removeOldestGhost() {
        NodePtr oldestGhost = ghostHead_->_next;
        if (oldestGhost != ghostTail_) {
            removeFromGhost(oldestGhost);
            ghostCache_.erase(oldestGhost->getKey());
        }
    }
private:
    size_t capacity_;
    size_t ghostCapacity_;
    size_t transformThreshold_;
    size_t minFreq_;
    std::mutex mutex_;

    NodeMap mainCache_;
    NodeMap ghostCache_;
    FreMap freMap_;

    NodePtr ghostHead_;
    NodePtr ghostTail_;
};