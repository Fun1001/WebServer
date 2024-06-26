/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-06-24 17:14:25
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-06-26 23:03:20
 * @FilePath: /WebServer/test/test.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "mysql_connection.h"  // 或者具体的包含 PreparedStatement 的头文件
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/exception.h>

// #include "../code/pool/threadpool.h"
#include <cstddef>
#include <features.h>
#include <memory>
#include <thread>

#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

void TestLog(){
    int cnt = 0, level = 0;
    //同步写日志测试
    Log::GetInstance()->init(level, "./testlog1", ".log", 0);
    for(level = 3; level >= 0; level--){
        Log::GetInstance()->SetLevel(level);
        for(int j = 0; j < 1000; j++){
            for(int i = 0; i < 4; i++){
                LOG_BASE(level, "%s 1111111111 %d ==================", "Test", cnt++);
            }
        }
    }

    cnt = 0;
    Log::GetInstance()->init(level, "./testlog2", ".log", 5000);
    for(level = 0; level < 4; level++) {
        Log::GetInstance()->SetLevel(level);
        for(int j = 0; j < 1000; j++ ){
            for(int i = 0; i < 4; i++) {
                LOG_BASE(i,"%s 222222222 %d ============= ", "Test", cnt++);
            }
        }
    }
}

void TestSqlLog(){
    Log::GetInstance()->init(0, "./testSqlpool", ".log", 5000);
    std::shared_ptr<SqlConnPool> _pool = nullptr;
    _pool = std::make_shared<SqlConnPool>("127.0.0.1:3306", "root", "123456", "testWebDB", std::thread::hardware_concurrency());
    // auto con = _pool->getConn();
    // try {
	// 	if(con == nullptr) {
	// 		return;
	// 	}

	// 	//准备查询语句
	// 	std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT passwd FROM user WHERE username = ?"));

	// 	//绑定参数
		
    //     pstmt->setString(1, "testName");

	// 	//执行查询
	// 	std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

	// 	//遍历结果集
	// 	while (res->next()) {
	// 		std::cout << "Check Email: " << res->getString("passwd") << std::endl;
	// 		if("123" != res->getString("passwd")) {
	// 			_pool->returnConn(std::move(con));
	// 			return;
	// 		}
	// 		_pool->returnConn(std::move(con));
	// 		return;
	// 	}
	// 	return;
	// }
	// catch (sql::SQLException& e) {
	// 	_pool->returnConn(std::move(con));
	// 	std::cerr << "SQLException: " << e.what();
	// 	std::cerr << "(mysql errorcode: " << e.getErrorCode();
	// 	std::cerr << ", SQLState: " << e.getSQLState() << std::endl;
	// 	return;
	// }

}

int main(){
    // TestLog();
    TestSqlLog();
}