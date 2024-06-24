#pragma once
#include <memory>
#include <iostream>
#include <mutex>
using namespace std;

template<typename T>
class Singleton{
protected: 
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton operator=(const Singleton<T> st) =delete;

    static std::shared_ptr<T> _instance;

public:
    static std::shared_ptr<T> GetInstance(){
        static std::once_flag s_flag;
        std::call_once(s_flag, [&](){
            //直接使用 new T() 而不是 std::make_shared<T>()。
            //这是因为 std::make_shared 需要访问 T 的构造函数
            _instance = std::shared_ptr<T>(new T());
        });
        return _instance;
    }

    virtual ~Singleton(){
        std::cout << "this is singleton destruct" << std::endl;
    }
};

template<typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;
