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
            _instance = std::make_shared<T>();
        });

        return _instance;
    }

    ~Singleton(){
        std::cout << "this is singleton destruct" << std::endl;
    }
};

template<typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;