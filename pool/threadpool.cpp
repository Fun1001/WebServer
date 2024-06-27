/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-06-27 21:25:13
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-06-27 21:42:53
 * @FilePath: /WebServer/pool/threadpool.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "threadpool.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>


ThreadPool::~ThreadPool()
{
    if (static_cast<bool>(_pool)) {
        std::lock_guard<std::mutex> lock(_pool->_mutex);
        _pool->_b_close = true;
    }
    _pool->_cond.notify_all();
}

ThreadPool::ThreadPool(size_t poolSize): _pool(std::make_shared<Pool>())
{
    assert(poolSize > 0);
    for(size_t i = 0; i < poolSize; ++i){
        std::thread([pool = _pool]{
            std::unique_lock<std::mutex> lock(pool->_mutex);
            while (true) {
                if(!pool->tasks.empty()){
                    auto task = std::move(pool->tasks.front());
                    pool->tasks.pop();
                    lock.unlock();
                    task();
                    lock.lock();
                }
                else if (pool->_b_close) {
                    break;
                }
                else {
                    pool->_cond.wait(lock);
                }
            }
        }).detach();
    }
}
