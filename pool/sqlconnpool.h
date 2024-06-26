/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-06-24 20:38:01
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-06-26 21:48:54
 * @FilePath: /WebServer/pool/sqlconnpool.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __SQLCONNPOOL_H__
#define __SQLCONNPOOL_H__

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mysql_driver.h>
#include <string>
#include <queue>
#include <mutex>
//#include <semaphore.h> //信号量（Semaphore）是一种用于线程同步和进程间同步的机制。这里我们使用条件变量代替
#include <thread>
#include "../log/log.h"
#include "../RAII/Defer.h"

class SqlConnection
{
public:
	SqlConnection(sql::Connection* con, int64_t lastime) : _con(con), _last_oper_time(lastime){}
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_oper_time;
};

class SqlConnPool
{
public:
    SqlConnPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize);
    ~SqlConnPool();
    void checkConnection();
    std::unique_ptr<SqlConnection> getConn();
    void returnConn(std::unique_ptr<SqlConnection> conn);
    int getFreeConnCount();

    void Close();
private:
    int _userCount;
    int _freeCount;
    std::string _url;
	std::string _user;
	std::string _pass;
	std::string _schema;
	int _poolSize;

    std::queue<std::unique_ptr<SqlConnection>> _connPool;
    std::mutex _mutex;
    std::condition_variable _cond;
    std::atomic<bool> _b_stop;

    std::thread _check_thread;//数据库保活线程
};

#endif // __SQLCONNPOOL_H__