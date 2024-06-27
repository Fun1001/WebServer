/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-06-27 14:18:51
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-06-27 16:55:28
 * @FilePath: /WebServer/timer/HeadTimer.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include "headtimer.h"
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>


HeadTimer::HeadTimer()
{
    _heap.reserve(64);
}

HeadTimer::~HeadTimer()
{
    clear();
}

void HeadTimer::adjust(int id, int newExpires)
{
    /* 调整指定id的结点 */
    assert(!_heap.empty() && _ref.count(id) > 0);
    _heap[_ref[id]].expires = Clock::now() + MS(newExpires);
    _siftdown(_ref[id], _heap.size());
}

void HeadTimer::add(int id, int timeOut, const TimeoutCallBack& cb)
{
    assert(id >= 0);
    size_t i;
    if (_ref.count(id) == 0) {
    // 新节点
        i = _heap.size();
        _ref[id] = i;
        _heap.push_back({id, Clock::now() + MS(timeOut), cb});
        _siftup(i);
    }
    else {
    //旧节点
        i = _ref[id];
        _heap[i].expires = Clock::now() + MS(timeOut);
        _heap[i].cb = cb;
        if(!_siftdown(i, _heap.size())){
            _siftup(i);
        }
    }
}

void HeadTimer::doWork(int id)
{
    /* 删除指定id结点，并触发回调函数 */
    if(_heap.empty() || _ref.count(id) == 0){
        return;
    }
    size_t i = _ref[id];
    TimerNode node = _heap[i];
    node.cb();
    _del(i);
}

void HeadTimer::clear()
{
    _ref.clear();
    _heap.clear();
}

void HeadTimer::tick()
{
    /* 清除超时结点 */
    if(_heap.empty()){
        return;
    }
    while (!_heap.empty()) {
        TimerNode node = _heap.front();
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0){
            break;
        }
        node.cb();
        pop();
    }
}

void HeadTimer::pop()
{
    assert(!_heap.empty());
    _del(0);
}

int HeadTimer::GetNextTick()
{
    tick();
    size_t res = -1;
    if(!_heap.empty()){
        res = std::chrono::duration_cast<MS>(_heap.front().expires - Clock::now()).count();
        if(res < 0){
            res = 0;
        }
    }
    return res;
}

void HeadTimer::_del(size_t index)
{
    /* 删除指定位置的结点 */
    assert(!_heap.empty() && index >= 0 && index < _heap.size());
    /* 将要删除的结点换到队尾，然后调整堆 */
    size_t i = index;
    size_t n = _heap.size() - 1;
    assert(i <= n);
    if(i < n){
        _SwapNode(i, n);
        if(!_siftdown(i, n)){
            _siftup(i);
        }
    }
    _ref.erase(_heap.end()->id);
    _heap.pop_back();
}

void HeadTimer::_siftup(size_t i)
{
    assert(i >= 0 && i < _heap.size());
    //获取父节点j
    size_t j = (static_cast<int>(i) - 1) / 2;
    while (i > 0) {
        //比较过期时间expires的大小
        if (_heap[j] < _heap[i]) {
            break;
        }
        _SwapNode(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

bool HeadTimer::_siftdown(size_t index, size_t n)
{
    assert(index >= 0 && index < _heap.size());
    assert(n >= 0 && n <= _heap.size());
    size_t i = index;
    // 左子节点
    size_t j = i * 2 + 1;
    while (j < n) {
        if(j + 1 && _heap[j+1]<_heap[j]){
            // 如果右子节点存在且小于左子节点，选择右子节点作为交换对象。
            j++;
        }
        if (_heap[i] < _heap[j]) {
            break;
        }
        _SwapNode(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

void HeadTimer::_SwapNode(size_t i, size_t j)
{
    assert(i >= 0 && i < _heap.size());
    assert(j >= 0 && j < _heap.size());

    std::swap(_heap[i], _heap[j]);
    _ref[_heap[i].id] = j;
    _ref[_heap[j].id] = i;
}
