/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-06-27 17:21:54
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-06-27 21:25:47
 * @FilePath: /WebServer/pool/threadpool.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <cstddef>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
#include <utility>

class ThreadPool{
public:
    explicit ThreadPool(size_t poolSize);

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;

    ~ThreadPool();

    template<class T>
    void AddTask(T&& task){
        {
            std::lock_guard<std::mutex> lock(_pool->_mutex);
            _pool->tasks.emplace(std::forward<T>(task));//完美转发
        }
        _pool->_cond.notify_one();
    }


private:
    struct Pool{
        std::mutex _mutex;
        std::condition_variable _cond;
        bool _b_close;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> _pool;
};
#endif // __THREADPOOL_H__