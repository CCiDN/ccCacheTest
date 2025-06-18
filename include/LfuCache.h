#pragma once

#include <caChePolicy.h>

#include <memory>
#include <mutex>
#include <unordered_map>


template<typename Key, typename Value> class LfuCache;

template<typename Key, typename Value>
class FreqList{
private:
    struct Node{
        int freq;
        Key _key;
        Value _value;
        std::weak_ptr<Node> _pre;
        std::shared_ptr<Node> _next;
        Node()
        :freq(1), _next(nullptr){}
        Node(Key key, Value value)
        :freq(1), _key(key), _value(value), _next(nullptr) {}
    };
    int _freq;
    using NodePtr = std::shared_ptr<Node>;
    NodePtr _head;
    NodePtr _tail;

public:
    explicit FreqList(int n)
    : _freq(n)
    {
        _head = std::make_shared<Node>();
        _tail = std::make_shared<Node> ();
        _head->_next = _tail;
        _tail->_pre = _head;
    }
    bool isEmpty(){
        return _head->_next == _tail;
    }
    void addNode(NodePtr node){
        if (!node || !_head || !_tail)
            return;
        node->_pre = _tail->_pre;
        node->_next = _tail;
        _tail->_pre.lock()->_next = node;
        _tail->_pre = node;
    }
    void removeNode(NodePtr node){
        if (!node || !_head || !_tail) return;
        if (node->_pre.expired() || !node->_next) return;
        node->_pre.lock()->_next = node->_next;
        node->_next->_pre = node->_pre;
        node->_next = nullptr;
    }
    NodePtr getFirstNode() const {return _head->_next;}
    friend class LfuCache<Key, Value>;
};

template<typename Key, typename Value>
class LfuCache : public caChepolicy<Key, Value>{
public:
    using Node = typename FreqList<Key, Value>::Node;
    using NodePtr = std::shared_ptr<Node>;
    using NodeMap = std::unordered_map<Key, NodePtr>;

    LfuCache(int n, int maxAverageNum = 10)
    :_capacity(n),_maxAverageNum(maxAverageNum),_minFreq(INT8_MAX)
    ,_curTotalNum(0),_curAverageNum(0)
    {}
    ~LfuCache() override = default;
    void put(Key _key,const Value& _value) override {
        if (_capacity == 0) return;
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _nodeMap.find(_key);
        if (it != _nodeMap.end()){
            it->second->_value = _value;
            Value dummyVal = _value;
            getInternal(it->second, dummyVal);
            return;
        }
        putInternal(_key, _value);
    }
    bool get(Key _key, Value& _value) override {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _nodeMap.find(_key);
        if (it != _nodeMap.end()){
            getInternal(it->second, _value);
            return true;
        }
        return false;
    }
    Value get(Key _key) override {
        Value _value{};
        this->get(_key, _value);
        return _value;
    }
private:
    void putInternal(Key key,const Value& value); // 添加缓存
    void getInternal(NodePtr node,Value& value); // 获取缓存(update)

    void kickOut(); // 移除缓存中的过期数据

    void removeFromFreqList(NodePtr node); // 从频率列表中移除节点
    void addToFreqList(NodePtr node); // 添加到频率列表

    void addFreqNum(); // 增加平均访问等频率
    void decreaseFreqNum(int num); // 减少平均访问等频率
    void handleOverMaxAverageNum(); // 处理当前平均访问频率超过上限的情况
    void updateMinFreq();

private:
    int  _capacity; // 缓存容量
    int  _minFreq; // 最小访问频次(用于找到最小访问频次结点)
    int  _maxAverageNum; // 最大平均访问频次
    int  _curAverageNum; // 当前平均访问频次
    int  _curTotalNum; // 当前访问所有缓存次数总数 
    std::mutex  _mutex; // 互斥锁
    NodeMap  _nodeMap; // key 到 缓存节点的映射
    std::unordered_map<int, std::shared_ptr<FreqList<Key, Value>>> _freqToFreqList;;// 访问频次到该频次链表的映射
};

#include "../src/LfuCache.tpp"