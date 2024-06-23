#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <cassert>
#include <chrono>
#include <cstddef>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>

template<class T>
class BlockDeque{
public:
     /*初始化阻塞队列*/
    explicit BlockDeque(std::size_t MaxCapacity = 1000);
    //析构
    ~BlockDeque();
    //清空队列
    void clear();
    //判断队列是否为空
    bool empty();
    //判断队列是否已满
    bool full();
    //关闭队列的进出，会通知生产/消费者
    void Close();
    //获取队列当前的数据数目
    std::size_t size();
    //获取队列的最大负载
    std::size_t capacity();
    //获取队首元素
    T front();
    ///获取队尾元素
    T back();
     /*往队列中添加元素前需要先将所有使用队列的线程先唤醒*/
    /*阻塞队列封装了生产者消费者模型，调用push的是生产者，也就是工作线程*/
    void push_back(const T& item);

    void push_front(const T& item);
     /*调用pop的是消费者，负责把生产者的内容写入文件*/
    bool pop(T& item);

    bool pop(T& item, int timeout);
    //通知一个等待的消费者线程
    void flush();

private:
    std::deque<T> _deq;
    std::size_t _capacity;
    std::mutex _mutex;
    bool _b_isClose;
    std::condition_variable _condConsumer;
    std::condition_variable _conProducer;
};

//实现
template<class T>
BlockDeque<T>::BlockDeque(std::size_t MaxCapacity) :_capacity(MaxCapacity){
    assert(MaxCapacity > 0);
    _b_isClose = false;
}

template<class T>
BlockDeque<T>::~BlockDeque(){
    Close();
}

template<class T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> lock(_mutex);
    _deq.clear();
}

template<class T>
bool BlockDeque<T>::empty() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _deq.empty();
}

template<class T>
bool BlockDeque<T>::full() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _deq.size() >= _capacity;
}

template<class T>
void BlockDeque<T>::Close() {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _deq.clear();
        _b_isClose = true;
    }
    _condConsumer.notify_all();
    _conProducer.notify_all();
}

template<class T>
std::size_t BlockDeque<T>::size() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _deq.back();
}

template<class T>
std::size_t BlockDeque<T>::capacity() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _capacity;
}

template<class T>
T BlockDeque<T>::front() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _deq.front();
}

template<class T>
T BlockDeque<T>::back() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _deq.back();
}

template<class T>
void BlockDeque<T>::push_back(const T& item) {
    std::unique_lock<std::mutex> lock(_mutex);
    while (_deq.size() >= _capacity) {
        _conProducer.wait(lock);
    }
    _deq.push_back(item);
    _condConsumer.notify_one();
}

template<class T>
void BlockDeque<T>::push_front(const T& item) {
    std::unique_lock<std::mutex> lock(_mutex);
    while (_deq.size() >= _capacity) {
        _conProducer.wait(lock);
    }
    _deq.push_front(item);
    _condConsumer.notify_one();
}

template<class T>
bool BlockDeque<T>::pop(T& item) {
    std::unique_lock<std::mutex> lock(_mutex);
    while(_deq.empty()){
        if (_b_isClose) {
            return false;
        }
        _condConsumer.wait(lock);
    }
    item = _deq.front();
    _deq.pop_front();
    _conProducer.notify_one();
    return true;
}

template<class T>
bool BlockDeque<T>::pop(T& item, int timeout) {
    std::unique_lock<std::mutex> lock(_mutex);
    while (_deq.empty()) {
        if (_b_isClose) {
            return false;
        }
        if(_condConsumer.wait_for(lock, std::chrono::seconds(timeout)) == std::cv_status::timeout){
            return false;
        }
    }
    item = _deq.front();
    _deq.pop_front();
    _conProducer.notify_one();
    return true;
}

template<class T>
void BlockDeque<T>::flush() {
    _condConsumer.notify_one();
}

#endif // BLOCKQUEUE_H