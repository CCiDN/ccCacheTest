
#include <LfuCache.h>
#include <climits>

template<typename Key, typename Value>
void LfuCache<Key, Value>::getInternal(NodePtr node,Value& value){
    value = node->_value;
    removeFromFreqList(node);
    int oldFreq = node->freq;
    node->freq = std::min(node->freq + 1, _maxAverageNum * 2);
    addToFreqList(node);
    if (_freqToFreqList.find(oldFreq) == _freqToFreqList.end() && node->freq == _minFreq + 1)
        _minFreq++;
    addFreqNum();
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::putInternal(Key _key,const Value& _value){
    if (_capacity == _nodeMap.size()) kickOut();
    NodePtr tempPtr = std::make_shared<Node>(_key, _value);
    _nodeMap[_key] = tempPtr;
    addToFreqList(tempPtr);
    addFreqNum();
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::kickOut(){
    updateMinFreq();
    NodePtr tempNode = _freqToFreqList[_minFreq]->getFirstNode();
    removeFromFreqList(tempNode);
    _nodeMap.erase(tempNode->_key);
    decreaseFreqNum(tempNode->freq);
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::removeFromFreqList(NodePtr node){
    if (!node) return;
    int freq = node->freq;
    _freqToFreqList[freq]->removeNode(node);
    if (_freqToFreqList[freq]->isEmpty()) {
        _freqToFreqList.erase(freq);
    }
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::addToFreqList(NodePtr node){
    if (!node) return;
    int freq = node->freq;
    if (_freqToFreqList.find(freq) == _freqToFreqList.end()){
        _freqToFreqList[freq] = std::make_shared<FreqList<Key, Value>>(freq);
    }
    _freqToFreqList[freq]->addNode(node);
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::addFreqNum(){
    _curTotalNum++;

    if (_nodeMap.empty())
        _curAverageNum = 0;
    else
        _curAverageNum = _curTotalNum / _nodeMap.size();
    
    if (_curAverageNum > _maxAverageNum)
        handleOverMaxAverageNum();
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::decreaseFreqNum(int num){
    _curTotalNum -= num;
    if (_nodeMap.empty())
        _curAverageNum = 0;
    else
        _curAverageNum  = _curTotalNum / _nodeMap.size();
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::handleOverMaxAverageNum(){
    if (_nodeMap.empty()) return;
    for (auto it = _nodeMap.begin(); it != _nodeMap.end(); ++it){
        if (!it->second) continue;

        NodePtr tempPtr = it->second;
        if (tempPtr->freq > _maxAverageNum / 2) {
            removeFromFreqList(tempPtr);
            tempPtr->freq /= 2;
            if (tempPtr->freq < 1) tempPtr->freq = 1;
            addToFreqList(tempPtr);
        }
    }
    updateMinFreq();
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::updateMinFreq(){
    _minFreq = INT_MAX;
    for (const auto& [freq, list] : _freqToFreqList) {
        if (list && !list->isEmpty()) {
            _minFreq = std::min(_minFreq, freq);
        }
    }
    if (_minFreq == INT_MAX) _minFreq = 1;
}