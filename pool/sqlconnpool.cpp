/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-06-24 20:41:16
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-07-04 16:00:54
 * @FilePath: /WebServer/pool/sqlconnpool.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include "sqlconnpool.h"
#include <cassert>
#include <chrono>
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <memory>
#include <mutex>
#include <mysql_driver.h>
#include <thread>
#include <utility>
#include "cppconn/statement.h"

void SqlConnPool::checkConnection()
{
    std::lock_guard<std::mutex> lock(_mutex);
    int poolSize = _connPool.size();
    //获取当前时间戳
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    //转换为秒
    long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
    for (int  i = 0; i < poolSize; i++)
    {
        auto con = std::move(_connPool.front());
        _connPool.pop();
        
        Defer defer([this, &con](){
            _connPool.push(std::move(con));
        });

        if(timestamp - con->_last_oper_time < 600){
            continue;
        }
        
        //更新连接
        try {
            std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
            stmt->executeQuery("SELECT 1");
            con->_last_oper_time = timestamp;
            LOG_INFO("execute timer alive query ,cur is %d", timestamp);
        } catch (sql::SQLException& e) {
            LOG_ERROR("Error keeping connection alive: %s", e.what());
			//创建新的连接 并替换
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
			auto* newcon = driver->connect(_url, _user, _pass);
			newcon->setSchema(_schema);
			con->_con.reset(newcon);
			con->_last_oper_time = timestamp;
        }
    }
    
}

std::unique_ptr<SqlConnection> SqlConnPool::getConn()
{
    std::unique_lock<std::mutex> lock(_mutex);
    //返回false条件变量挂起，线程等待
    _cond.wait(lock, [this](){
        if(_b_stop){
            return true;
        }
        return !_connPool.empty();
    });
    if (_b_stop) {
        return nullptr;
    }
    std::unique_ptr<SqlConnection> con(std::move(_connPool.front()));
    _connPool.pop();

    return con;
}

void SqlConnPool::returnConn(std::unique_ptr<SqlConnection> conn)
{
    std::unique_lock<std::mutex> lock(_mutex);
    if(_b_stop){
        return;
    }
    _connPool.push(std::move(conn));
    _cond.notify_one();
}

int SqlConnPool::getFreeConnCount()
{
    std::lock_guard<std::mutex> lock(_mutex);

    return _connPool.size();
}

void SqlConnPool::Close()
{
    _b_stop = true;
    _cond.notify_all();
}

SqlConnPool::SqlConnPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize):
    _userCount(0), _freeCount(0), _url(url), _user(user), _pass(pass), _schema(schema), _poolSize(poolSize)
{
    assert(_poolSize > 0);

    try {
        //创建连接
        for(int i = 0; i<_poolSize; ++i){
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            auto conn = driver->connect(_url, _user, _pass);
            conn->setSchema(_schema);
            //获取当前时间戳
            auto currentTime = std::chrono::system_clock::now().time_since_epoch();
            //转换为秒
            long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
            _connPool.push(std::make_unique<SqlConnection>(conn, timestamp));
        }

        //数据库保活线程
        _check_thread = std::thread([this](){
            while (!_b_stop) {
                checkConnection();
                std::this_thread::sleep_for(std::chrono::seconds(300));
            }
        });
        _check_thread.detach();
    } catch (sql::SQLException& e) {
        LOG_ERROR("Error create connection : %s", e.what());
    }
}

SqlConnPool::~SqlConnPool()
{
    std::unique_lock<std::mutex> lock(_mutex);
    while (!_connPool.empty()) {
        _connPool.pop();
    }
}

bool MysqlDao::UserVerify(const string &name, const string &pwd, bool isLogin){
    auto con = _pool->getConn();
    if (con == nullptr) {
        return false;
    }

    Defer defer([this, &con](){
        _pool->returnConn(std::move(con));
    });

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE username = ?"));
        pstmt->setString(1,name);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        std::string origin_pwd = "";
        // 遍历结果集
		while (res->next()) {
			origin_pwd = res->getString("pwd");
			// 输出查询到的密码
			LOG_INFO("pwd is: %s", origin_pwd.c_str());
			break;
		}

        if (pwd != origin_pwd) {
            return  false;
        }
        return true;
    } catch (sql::SQLException& e) {
        LOG_ERROR("SQLException: %s", e.what());
        LOG_ERROR("MySQL error code: : %d", e.getErrorCode());
        LOG_ERROR("SQLState: %s", e.getSQLState().c_str());
		return false;
    }
}

bool MysqlDao::Regirster(const string &name, const string &pwd){
    auto con = _pool->getConn();
	try {
		if (con == nullptr) {
			return false;
		}

        Defer defer([this, &con](){
        _pool->returnConn(std::move(con));
        });
		// 准备调用存储过程
		std::unique_ptr<sql::PreparedStatement> stmt(con->_con->prepareStatement("CALL reg_user(?,?,@result)"));
		// 设置输入参数
		stmt->setString(1, name);
		stmt->setString(2, pwd);

		// 由于PreparedStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值

		  // 执行存储过程
		stmt->execute();
		// 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
	    // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
	    unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
	    unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));
	  if (res->next()) {
        int result = res->getInt("result");
        return result;
	  }
        return false;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

MysqlDao::~MysqlDao() {
    // 析构函数实现，如果不需要特殊操作，可以留空
}