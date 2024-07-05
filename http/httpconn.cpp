#include "httpconn.h"
#include "httprequest.h"
#include <arpa/inet.h>
#include <bits/types/struct_iovec.h>
#include <cassert>
#include <cstdint>
#include <memory>
#include <sys/uio.h>
#include <unistd.h>

HttpConn::HttpConn(): _fd(-1), _addr({0}), _isClose(true)
{
    
}

HttpConn::~HttpConn()
{
    Close();
}

void HttpConn::init(int sockFd, const sockaddr_in& addr)
{
    assert(sockFd >0);
    userCount++;
    _addr = addr;
    _fd = sockFd;
    _writeBuff.RetrieveAll();
    _readBuff.RetrieveAll();
    _isClose = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", _fd, GetIP(), GetPort(), (int)userCount);
}

ssize_t HttpConn::read(int* saveErrno)
{
    ssize_t len = -1;
    do {
        len = _readBuff.ReadFd(_fd, saveErrno);
        if(len < 0){
            break;
        }
    }while (isET);
    return len;
}

ssize_t HttpConn::write(int* saveErrno)
{
    ssize_t len = -1;
    do {
        len = writev(_fd, _iov, _iovCnt);
        if(len <= 0){
            *saveErrno = errno;
            break;
        }
        if(_iov[0].iov_len + _iov[1].iov_len == 0) {break;} //写完了
        // 如果写入的长度超过了第一个iovec的长度,调整第二个iovec的基地址和长度
        else if (static_cast<size_t>(len) > _iov[0].iov_len) {
            _iov[1].iov_base = (uint8_t*)_iov[1].iov_base + (len - _iov[0].iov_len);
            _iov[1].iov_len -= (len - _iov[0].iov_len);
            // 如果第一个iovec还有未写入的数据,可能是因为写操作被中断或是因为非阻塞I/O的限制,需要确保缓冲区被清空并重置，以便在下一次写入尝试时能够从正确的位置开始。
            if (_iov[0].iov_len) {
                _writeBuff.RetrieveAll();
                _iov[0].iov_len = 0;
            }
        }
        // 如果写入的长度没有超过第一个iovec的长度, 调整第一个iovec的基地址和长度
        else {
            _iov[0].iov_base = (uint8_t*)_iov[0].iov_base + len;
            _iov[0].iov_len -= len;
            _writeBuff.RetrieveAll();
        }
    }while (isET || ToWriteBytes() > 1024);
    return len;
}

void HttpConn::Close()
{
    _response->UnmapFile();
    if(!_isClose){
        _isClose = true;
        userCount--;
        close(_fd);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", _fd, GetIP(), GetPort(), (int)userCount);
    }
}

int HttpConn::GetFd() const
{
    return _fd;
}

int HttpConn::GetPort() const
{
    return _addr.sin_port;
}

const char* HttpConn::GetIP() const
{
    return inet_ntoa(_addr.sin_addr);
}

sockaddr_in HttpConn::GetAddr() const
{
    return _addr;
}

bool HttpConn::process()
{
    _request = std::make_shared<HttpRequest>();
    if(_readBuff.ReadableBytes() <=0){
        return false;
    }
    else if (_request->parse(_readBuff)) {
        LOG_DEBUG("%s", _request->getPath().c_str());
        _response->Init(srcDir, _request->getPath(), _request->isKeepAlive(), 200);
    }
    else {
        _response->Init(srcDir, _request->getPath(), false, 400);
    }

    _response->MakeResponse(_writeBuff);
    //响应头
    _iov[0].iov_base = const_cast<char*>(_writeBuff.Peek());
    _iov->iov_len = _writeBuff.ReadableBytes();
    _iovCnt = 1;
    //文件
    if(_response->FileLen() > 0 && _response->File()){
        _iov[1].iov_base = _response->File();
        _iov[1].iov_len = _response->FileLen();
        _iovCnt = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", _response->FileLen(), _iovCnt, ToWriteBytes());
    return true;
}

int HttpConn::ToWriteBytes()
{
    return _iov[0].iov_len + _iov[1].iov_len;
}

bool HttpConn::IsKeepAlive() const
{
    return _request->isKeepAlive();
}
