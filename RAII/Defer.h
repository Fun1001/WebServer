#include <iostream>
#include <functional>
#include <mutex>

class Defer{
public:
    // 接受一个lambda表达式或者函数指针
    Defer(std::function<void()> func) :_func(func), _active(true){}

    //移动构造
    Defer(Defer&& other) noexcept : _func(std::move(other._func)), _active(other._active){
        other._active = false;
    }

    // 禁止拷贝构造函数和拷贝赋值运算符
    Defer(const Defer&) = delete;
    Defer& operator=(const Defer&) = delete;

    // 析构函数中执行传入的函数
    ~Defer(){
        if(_active){
            std::lock_guard<std::mutex> lock(_mutex);
            if(_func){
                _func();
            }
        }
    }

    // 取消延迟执行
    void cancel() {
        std::lock_guard<std::mutex> lock(_mutex);
        _active = false;
    }
    
private:
    std::function<void()> _func; // 要延迟执行的函数对象
    bool _active;                // 控制是否执行函数对象
    std::mutex _mutex;           // 保护_active的互斥锁
};