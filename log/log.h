/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-06-26 17:17:36
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-06-26 22:09:40
 * @FilePath: /WebServer/log/log.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef LOG_H
#define LOG_H

#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>           // vastart va_end
#include <assert.h>
#include <sys/stat.h>         //mkdir
#include <type_traits>
#include "block_queue.h"
#include "../Singleton/Singleton.h"
#include "../buffer/buffer.h"

class Log : public Singleton<Log> {
    friend class Singleton<Log>;

public:
    void init(int level, const char* path = "./log", const char* suffix = ".log", int maxQueueCapacity = 1024);

    static void FlushLogThread();

    void write(int level, const char* format,...);
    void flush();
    
    int GetLevel();
    void SetLevel(int level);

    bool IsOpen() {return _b_isOpen;}

    virtual ~Log();

protected:
    
    Log();

private:
    
    void AppendLogLevelTitle_(int level);
    void AsyncWrite();

private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* _path;
    const char* _suffix;

    int _MAX_LINES;

    int _lineCount;                                     //日志行数记录
    int _toDay;                                         //日志按天分类，记录当前是哪一天
    bool _b_isOpen;

    Buffer _buff;
    int _level;
    bool _b_isAsync;                                     //是否是异步           

    FILE* _fp;                                          //打开log的文件指针
    std::unique_ptr<BlockDeque<std::string>> _deque;    //阻塞队列
    std::unique_ptr<std::thread> _writeThread;          
    std::mutex _mutex;                      
};

#define LOG_BASE(level, format, ...) \
    do {\
        std::shared_ptr<Log> log = Log::GetInstance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif