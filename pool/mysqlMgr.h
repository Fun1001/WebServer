/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-06-28 17:53:30
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-07-04 15:03:38
 * @FilePath: /WebServer/pool/mysqlMgr.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __MYSQLMGR_H__
#define __MYSQLMGR_H__
#include "sqlconnpool.h"
class MysqlMgr: public Singleton<MysqlMgr>
{
	friend class Singleton<MysqlMgr>;
public:
	~MysqlMgr() {};
    bool UserVerify(const string &name, const string &pwd, bool isLogin){
        return _dao.UserVerify(name, pwd, isLogin);
    }
	bool Regirster(const string &name, const string &pwd){
		return _dao.Regirster(name, pwd);
	}
private:
	MysqlMgr() {};
	MysqlDao  _dao;
};
#endif // __MYSQLMGR_H__