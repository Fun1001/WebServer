/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-07-05 16:11:34
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-07-05 16:59:08
 * @FilePath: /WebServer/http/httpconn.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __HTTPCONN_H__
#define __HTTPCONN_H__

#include <atomic>
#include <bits/types/struct_iovec.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>      
#include "../log/log.h"
#include "../buffer/buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn {
public:
    HttpConn();

    ~HttpConn();

    void init(int sockFd, const sockaddr_in& addr);

    ssize_t read(int* saveErrno);

    ssize_t write(int* saveErrno);

    void Close();

    int GetFd() const;

    int GetPort() const;

    const char* GetIP() const;

    sockaddr_in GetAddr() const;

    bool process();

    int ToWriteBytes();

    bool IsKeepAlive() const;

    static bool isET;
    static const char* srcDir;
    static std::atomic_int userCount;

private:
    int _fd;
    struct sockaddr_in _addr;
    bool _isClose;
    int _iovCnt;
    struct iovec _iov[2];

    Buffer _readBuff;
    Buffer _writeBuff;

    HttpRequest* _request;
    HttpResponse* _response;
};
#endif // __HTTPCONN_H__