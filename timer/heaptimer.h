#ifndef __HEAPTIMER_H__
#define __HEAPTIMER_H__

#include <chrono>
#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>
typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;//TimeStamp 用于表示定时器的过期时间点。

struct TimerNode {
    int id; //定时器的唯一标识符
    TimeStamp expires; //定时器的过期时间点。
    TimeoutCallBack cb; //定时器超时回调函数
    
    bool operator<(const TimerNode& t){
        return expires < t.expires;
    }
};

class HeapTimer {
public:
    HeapTimer();

    ~HeapTimer();

    //调整已有定时器的过期时间。
    void adjust(int id, int newExpires); 

    //添加新的定时器或调整已有定时器的过期时间。
    void add(int id, int timeOut, const TimeoutCallBack& cb);

    //执行定时器的回调函数并删除该定时器。
    void doWork(int id); 

    // 清空所有定时器。
    void clear();

    // 除所有已过期的定时器，并执行它们的回调函数。
    void tick();
    
    // 删除堆顶的定时器节点。
    void pop();

    // 获取下一个定时器的时间差。提供一个接口获取到下一个超时定时器的剩余时间。
    int GetNextTick();

private:
    //删除堆中指定索引的定时器节点
    void _del(size_t index);

    // 从下往上调整堆，确保最小堆性质。
    void _siftup(size_t i);

    // 从上往下调整堆，确保最小堆性质。
    bool _siftdown(size_t index, size_t n);

    // 交换堆中两个节点的位置，并更新映射。
    void _SwapNode(size_t i, size_t j); 

    std::vector<TimerNode> _heap; //存储定时器节点，形成最小堆
    std::unordered_map<int , size_t> _ref; //将定时器ID映射到堆中的索引，方便快速查找和调整。
};


#endif // __HEAPTIMER_H__