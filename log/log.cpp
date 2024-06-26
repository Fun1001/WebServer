#include "log.h"
#include "block_queue.h"
#include <bits/types/struct_timeval.h>
#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <memory>
#include <mutex>
#include <semaphore.h>
#include <string>
#include <sys/select.h>
#include <sys/stat.h>
#include <thread>

Log::Log(): _lineCount(0), _b_isAsync(false), _writeThread(nullptr), _toDay(0), _fp(nullptr) {}

Log::~Log(){
    if(_writeThread && _writeThread->joinable()){
        while (!_deque->empty()) {
            _deque->flush();
        }
        _deque->Close();
        _writeThread->join();
    }
    if(_fp){
        std::lock_guard<std::mutex> lock(_mutex);
        flush();
        fclose(_fp);
    }
}

//获取日志等级
int Log::GetLevel() {
    std::lock_guard<mutex> locker(_mutex);
    return _level;
}

//设置日志等级
void Log::SetLevel(int level){
    std::lock_guard<std::mutex> lock(_mutex);
    _level = level;
}

//初始化
void Log::init(int level = 1, const char* path, const char* suffix, int maxQueueCapacity)
{
    _b_isOpen = true;
    _level = level;
    
    if(maxQueueCapacity > 0){
        _b_isAsync = true;
        if(!_deque){
            //创建阻塞队列
            std::unique_ptr<BlockDeque<std::string>> newDeque = std::make_unique<BlockDeque<std::string>>();
            _deque = std::move(newDeque);

            /*创建一个新线程，执行异步写入文件函数*/
            std::unique_ptr<std::thread> NewThread = std::make_unique<std::thread>(FlushLogThread);
            _writeThread = std::move(NewThread);
        }
    }
    else{
        _b_isAsync =false;
    }

    _lineCount = 0;

    /*创建strcut tm变量接收当下时间*/
    time_t timer = time(nullptr);
    struct tm* sysTime = localtime(&timer);
    struct tm t = *sysTime;
    _path = path;
    _suffix = suffix;

    // 格式化日志文件名
    char fileName[LOG_NAME_LEN] = {[0]=0};
    snprintf(fileName, LOG_NAME_LEN, "%s/%04d_%02d_%02d%s", 
        _path, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, _suffix);
    _toDay = t.tm_mday;
    
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _buff.RetrieveAll();
        if(_fp){
            flush();
            fclose(_fp);
        }

        _fp = fopen(fileName, "a");
        if(_fp == nullptr){
            mkdir(_path, 0777);
            _fp = fopen(fileName, "a");
        }
        assert(_fp != nullptr);
    }
}

void Log::write(int level, const char* format,...){
    struct timeval now = {0,0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm* sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list valist;

    //日志日期，行数
    /*如果是新的一天了，或者日志行数到上限了，创建新日志*/
    if(_toDay != t.tm_mday || (_lineCount && (_lineCount % MAX_LINES == 0))){
        std::unique_lock<std::mutex> uniquelock(_mutex);
        uniquelock.unlock();

        char newFile[LOG_NAME_LEN];
        char tail[36] = {[0] = 0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        //新的一天
        if(_toDay != t.tm_mday){
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", _path, tail, _suffix);
            _toDay = t.tm_mday;
            _lineCount = 0;
        }
        else {//日志写满了
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", _path, tail, (_lineCount  / MAX_LINES), _suffix);
        }

        uniquelock.lock();
        flush();
        fclose(_fp);
        _fp = fopen(newFile, "a");
        assert(_fp != nullptr);
    }

    {
        std::unique_lock<std::mutex> uniquelock(_mutex);
        _lineCount++;
        /*写每一行的开头格式*/
        int n = snprintf(_buff.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);

        _buff.HasWritten(n);
        AppendLogLevelTitle_(level);

        /*可变参数定义初始化，在vsprintf时使用，作用：输入具体的日志内容*/
        va_start(valist, format);
        int m = vsnprintf(_buff.BeginWrite(), _buff.WritableBytes(), format, valist);
        va_end(valist);
        
         /*加入换行和空格*/
        _buff.HasWritten(m);
        _buff.Append("\n\0", 2);

        /*决定是异步写还是同步写*/
        if(_b_isAsync && _deque && !_deque->full()){
            _deque->push_back(_buff.RetrieveAllToStr());
        }
        else {
            fputs(_buff.Peek(),_fp);
        }
        _buff.RetrieveAll();
    }
}

void Log::AppendLogLevelTitle_(int level) {
    switch (level) {
    case 0:
        _buff.Append("[debug]: ", 9);
        break;
    case 1:
        _buff.Append("[info]: ", 8);
        break;
    case 2:
        _buff.Append("[warn]: ", 8); //长度必须对应上
        break;
    case 3:
        _buff.Append("[error]: ", 9);
        break;
    default:
        _buff.Append("[info]: ", 9);
        break;
    }
}

//刷新日志
void Log::flush(){
    if(_b_isAsync){
        _deque->flush();
    }
    // 将缓冲区中的数据立即写入文件
    fflush(_fp);
}

void Log::AsyncWrite(){
    std::string str = "";
    /*循环从阻塞队列里获取资源*/
    while (_deque->pop(str)) {
        std::lock_guard<std::mutex> lock(_mutex);
        fputs(str.c_str(), _fp);//将c_str()输出到_fp指向的文件中
    }
}

void Log::FlushLogThread(){
    Log::GetInstance()->AsyncWrite();
}