#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <iomanip>
#include <random>
#include <algorithm>

#include "caChePolicy.h"
#include "LruCache.h"
#include "LruKCache.h"
#include "HashLruCache.h"
#include "LfuCache.h"
#include "HashLfuCache.h"
#include "ArcCache.h"

class Timer {
public:
    Timer(): start(std::chrono::high_resolution_clock::now()) {}
    double elapsed() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now-start).count();
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

void printResults(const std::string& testName, int capacity, 
                 const std::vector<int>& get_operations, 
                 const std::vector<int>& hits) {
    std::cout << "=== " << testName << " 结果汇总 ===" << std::endl;
    std::cout << "缓存大小: " << capacity << std::endl;     

    std::vector<std::string> names;
    if (hits.size() == 2) {
        names = {"LRU", "LRU-K"};
    } else if (hits.size() == 3) {
        names = {"LRU", "LRU-K", "HASHLRU"};
    } else if (hits.size() == 4) {
        names = {"LRU", "LRU-K", "HASHLRU", "LFU"};
    } else if (hits.size() == 5) {
        names = {"LRU", "LRU-K", "HASHLRU", "LFU", "LFU-k"};
    } else if (hits.size() == 6) {
        names = {"LRU", "LRU-K", "HASHLRU", "LFU", "LFU-k", "HASHLFU-k"};
    } else if (hits.size() == 7) {
        names = {"LRU", "LRU-K", "HASHLRU", "LFU", "LFU-k", "HASHLFU-k", "ARC"};
    } 
    for (size_t i = 0; i < hits.size(); ++i) {
        double hitRate = 100.0 * hits[i] / get_operations[i];
        std::cout << (i < names.size() ? names[i] : "Algorithm " + std::to_string(i+1)) 
                    << " - 命中率: " << std::fixed << std::setprecision(2) 
                    << hitRate << "% ";
        // 添加具体命中次数和总操作次数
        std::cout << "(" << hits[i] << "/" << get_operations[i] << ")" << std::endl;
    }           
}


void printResults(const std::string& testName, int capacity,
                    const int get_operations, const int hits) {
    std::cout << "=== " << testName << " 结果汇总 ===" << std::endl;
    std::cout << "缓存大小: " << capacity << std::endl;
    std::string name = "MyLru";
    double hitRate = 100.0 * hits / get_operations;
    std::cout << name << "-命中率: " << std::fixed << std::setprecision(2) << hitRate << "%";
    std::cout << "(" << hits << "/" << get_operations << ")" << std::endl;
}

void testHotDataAccess() {
    std::cout << "\n=== 测试场景1: 热点数据访问测试 ===" << std::endl;

    const int CAPACITY = 20;         // 缓存容量
    const int OPERATIONS = 50000;   // 总操作次数
    const int HOT_KEYS = 20;         // 热点数据数量
    const int COLD_KEYS = 5000;      // 冷数据数量

    LruCache<int, std::string> lru(CAPACITY);
    LruKCache<int, std::string> lruk(CAPACITY, 20, 2);
    HashLruCache<int, std::string> hashLru(CAPACITY, 2);
    LfuCache<int, std::string> lfu(CAPACITY,INT_MAX);
    LfuCache<int, std::string> lfuk(CAPACITY,30);
    HashLfuCache<int, std::string>hashlfuk(CAPACITY, 2, 30);
    ArcCahce<int, std::string>arc(CAPACITY, 5);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::array<caChepolicy<int, std::string>*, 7> caches = {&lru, &lruk, &hashLru, &lfu, &lfuk, &hashlfuk, &arc};
    std::vector<std::string> names = {"lru", "lruk", "hashLru", "lfu", "lfuk", "hashlfuk", "arc"};
    std::vector<int> hits(caches.size(), 0);
    std::vector<int> get_operations(caches.size(), 0);
    for (int i = 0; i < caches.size(); ++i){
        for (int key = 0; key < HOT_KEYS; ++key) {
            std::string value = "value" + std::to_string(key);
            caches[i]->put(key, value);
        }
        for (int op = 0; op < OPERATIONS; ++op) {
            // 大多数缓存系统中读操作比写操作频繁
            // 所以设置30%概率进行写操作
            bool isPut = (gen() % 100 < 30); 
            int key;
            
            // 70%概率访问热点数据，30%概率访问冷数据
            if (gen() % 100 < 70) {
                key = gen() % HOT_KEYS; // 热点数据
            } else {
                key = HOT_KEYS + (gen() % COLD_KEYS); // 冷数据
            }
            
            if (isPut) {
                // 执行put操作
                std::string value = "value" + std::to_string(key) + "_v" + std::to_string(op % 100);
                caches[i]->put(key, value);
            } else {
                // 执行get操作并记录命中情况
                std::string result;
                get_operations[i]++;
                if (caches[i]->get(key, result)) {
                    hits[i]++;
                }
            }
        }
    }
    printResults("热点数据访问测试", CAPACITY, get_operations, hits);
}

void testHotDataAccess(int) {
    std::cout << "\n=== 测试场景1: 热点数据访问测试(Same Data) ===" << std::endl;

    const int CAPACITY = 20;         // 缓存容量
    const int OPERATIONS = 50000;   // 总操作次数
    const int HOT_KEYS = 20;         // 热点数据数量
    const int COLD_KEYS = 5000;      // 冷数据数量

    LruCache<int, std::string> lru(CAPACITY);
    LruKCache<int, std::string> lruk(CAPACITY, 20, 2);
    HashLruCache<int, std::string> hashLru(CAPACITY, 2);
    LfuCache<int, std::string> lfu(CAPACITY, INT_MAX);
    LfuCache<int, std::string> lfuk(CAPACITY, 30);
    HashLfuCache<int, std::string>hashlfuk(CAPACITY, 2, 30);
    ArcCahce<int, std::string>arc(CAPACITY, 5);

    std::random_device rd;
    std::mt19937 gen(rd());

    // 所有策略共用一份操作序列
    struct Operation {
        bool isPut;
        int key;
    };
    std::vector<Operation> operations;
    operations.reserve(OPERATIONS);

    // 预热阶段：将所有热点数据写入缓存
    for (int key = 0; key < HOT_KEYS; ++key) {
        operations.push_back({true, key});
    }

    // 生成 OPERATIONS 个读写操作
    for (int i = 0; i < OPERATIONS; ++i) {
        bool isPut = (gen() % 100 < 30); // 30% put
        int key = (gen() % 100 < 70)
                    ? (gen() % HOT_KEYS)               // 热点
                    : (HOT_KEYS + gen() % COLD_KEYS);  // 冷点
        operations.push_back({isPut, key});
    }

    // 所有缓存策略
    std::array<caChepolicy<int, std::string>*, 7> caches = {&lru, &lruk, &hashLru, &lfu, &lfuk, &hashlfuk, &arc};
    std::vector<std::string> names = {"lru", "lruk", "hashLru", "lfu", "lfuk", "hashlfuk", "arc"};
    std::vector<int> hits(caches.size(), 0);
    std::vector<int> get_operations(caches.size(), 0);

    for (int i = 0; i < caches.size(); ++i) {
        for (int op_index = 0; op_index < operations.size(); ++op_index) {
            const auto& op = operations[op_index];

            if (op.isPut) {
                std::string value = "val" + std::to_string(op.key) + "_" + std::to_string(op_index % 100);
                caches[i]->put(op.key, value);
            } else {
                std::string result;
                get_operations[i]++;
                if (caches[i]->get(op.key, result)) {
                    hits[i]++;
                }
            }
        }
    }
    printResults("热点数据访问测试", CAPACITY, get_operations, hits);
}

void testLoopPattern(int) {
    std::cout << "\n=== 测试场景2：循环扫描测试 ===" << std::endl;

    const int CAPACITY = 50;
    const int LOOP_SIZE = 500;
    const int OPERATIONS = 200000;

    LruCache<int, std::string> lru(CAPACITY);
    LruKCache<int, std::string> lruk(CAPACITY, 20, 2);
    HashLruCache<int, std::string> hashLru(CAPACITY, 2);
    LfuCache<int, std::string> lfu(CAPACITY, INT_MAX);
    LfuCache<int, std::string> lfuk(CAPACITY, 30);
    HashLfuCache<int, std::string>hashlfuk(CAPACITY, 2, 30);
    ArcCahce<int, std::string>arc(CAPACITY, 5);

    std::array<caChepolicy<int, std::string>*, 7> caches = {&lru, &lruk, &hashLru, &lfu, &lfuk, &hashlfuk, &arc};
    std::vector<std::string> names = {"lru", "lruk", "hashLru", "lfu", "lfuk", "hashlfuk", "arc"};
    std::vector<int> hits(caches.size(), 0);
    std::vector<int> get_operations(caches.size(), 0);

    std::random_device rd;
    std::mt19937 gen(rd());

    // ✅ 操作序列结构（仅记录是否为 put 和 key）
    struct Operation {
        bool isPut;
        int key;
        int opIndex;  // 可用于生成 value
    };
    std::vector<Operation> operations;
    operations.reserve(OPERATIONS);

    // ✅ 提前生成操作序列，确保一致性
    int current_pos = 0;
    for (int op = 0; op < OPERATIONS; ++op) {
        bool isPut = (gen() % 100 < 20);  // 20% 写操作
        int key;

        if (op % 100 < 60) {
            key = current_pos;
            current_pos = (current_pos + 1) % LOOP_SIZE;
        } else if (op % 100 < 90) {
            key = gen() % LOOP_SIZE;
        } else {
            key = LOOP_SIZE + (gen() % LOOP_SIZE);
        }

        operations.push_back({isPut, key, op});
    }

    // ✅ 各缓存策略运行相同操作序列
    for (int i = 0; i < caches.size(); ++i) {
        // 预热：加载20%初始数据
        for (int key = 0; key < LOOP_SIZE / 5; ++key) {
            std::string value = "init" + std::to_string(key);
            caches[i]->put(key, value);
        }

        for (const auto& op : operations) {
            if (op.isPut) {
                // 即时生成写入 value，避免存储
                std::string value = "val_" + std::to_string(op.key) + "_" + std::to_string(op.opIndex % 100);
                caches[i]->put(op.key, value);
            } else {
                std::string result;
                get_operations[i]++;
                if (caches[i]->get(op.key, result)) {
                    hits[i]++;
                }
            }
        }
    }

    printResults("循环扫描测试", CAPACITY, get_operations, hits);
}

void testWorkloadShift(int) {
    std::cout << "\n=== 测试场景3：工作负载剧烈变化测试 ===" << std::endl;

    const int CAPACITY = 30;
    const int OPERATIONS = 80000;
    const int PHASE_LENGTH = OPERATIONS / 5;

    LruCache<int, std::string> lru(CAPACITY);
    LruKCache<int, std::string> lruk(CAPACITY, 100, 2);
    HashLruCache<int, std::string> hashLru(CAPACITY, 2);
    LfuCache<int, std::string> lfu(CAPACITY, INT_MAX);
    LfuCache<int, std::string> lfuk(CAPACITY, 30);
    HashLfuCache<int, std::string>hashlfuk(CAPACITY, 2, 30);
    ArcCahce<int, std::string>arc(CAPACITY, 5);

    std::array<caChepolicy<int, std::string>*, 7> caches = {&lru, &lruk, &hashLru, &lfu, &lfuk, &hashlfuk, &arc};
    std::vector<std::string> names = {"lru", "lruk", "hashLru", "lfu", "lfuk", "hashlfuk", "arc"};
    std::vector<int> hits(caches.size(), 0);
    std::vector<int> get_operations(caches.size(), 0);

    std::random_device rd;
    std::mt19937 gen(rd());

    // 保存操作序列：isPut + key（不保存 value）
    struct Operation {
        bool isPut;
        int key;
        int phase;
    };
    std::vector<Operation> operations;

    // 生成统一操作序列
    for (int op = 0; op < OPERATIONS; ++op) {
        int phase = op / PHASE_LENGTH;
        int putProbability;
        switch (phase) {
            case 0: putProbability = 15; break;
            case 1: putProbability = 30; break;
            case 2: putProbability = 10; break;
            case 3: putProbability = 25; break;
            case 4: putProbability = 20; break;
            default: putProbability = 20;
        }

        bool isPut = (gen() % 100 < putProbability);

        int key;
        if (op < PHASE_LENGTH) {
            key = gen() % 5;
        } else if (op < PHASE_LENGTH * 2) {
            key = gen() % 400;
        } else if (op < PHASE_LENGTH * 3) {
            key = (op - PHASE_LENGTH * 2) % 100;
        } else if (op < PHASE_LENGTH * 4) {
            int locality = (op / 800) % 5;
            key = locality * 15 + (gen() % 15);
        } else {
            int r = gen() % 100;
            if (r < 40) {
                key = gen() % 5;
            } else if (r < 70) {
                key = 5 + (gen() % 45);
            } else {
                key = 50 + (gen() % 350);
            }
        }

        operations.push_back({isPut, key, phase});
    }

    // 对每种策略执行相同操作序列
    for (int i = 0; i < caches.size(); ++i) {
        // 预热
        for (int key = 0; key < 30; ++key) {
            std::string value = "init" + std::to_string(key);
            caches[i]->put(key, value);
        }

        // 重放操作序列
        for (const auto& op : operations) {
            if (op.isPut) {
                std::string value = "value" + std::to_string(op.key) + "_p" + std::to_string(op.phase);
                caches[i]->put(op.key, value);
            } else {
                std::string result;
                get_operations[i]++;
                if (caches[i]->get(op.key, result)) {
                    hits[i]++;
                }
            }
        }
    }

    printResults("工作负载剧烈变化测试", CAPACITY, get_operations, hits);
}



int main(){
    testHotDataAccess(1);
    testLoopPattern(1);
    testWorkloadShift(1);
    return 0;
}

