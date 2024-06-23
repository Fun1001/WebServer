#include "buffer.h"
#include <algorithm>
#include <bits/types/struct_iovec.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <strings.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

Buffer::Buffer(int initBuffSize) : _buffer(initBuffSize), _readPos(0), _writePos(0){}

//可写的字节数
size_t Buffer::WritableBytes() const
{
    return _buffer.size() - _writePos;
}

//可读字节数
size_t Buffer::ReadableBytes() const 
{
    return _writePos - _readPos; //小于0时表示，读取的数据超过写入的数据，意味着缓冲区无数据可读
}

//返回缓冲区中可前置字节的大小，可前置字节是指在读取位置之前的空间，之前的是已经读过的，可弃之？
size_t Buffer::PrependableBytes() const
{
    return _readPos;
}

// 返回当前可读数据的起始位置的指针，但不改变读取位置。
const char* Buffer::Peek() const
{
    return _BeginPtr() + _readPos;
}

//确保有足够的可写空间， 不够就扩容？
void Buffer::EnsureWritable(size_t len)
{
    if(WritableBytes() < len){
        _MakeSpace(len);
    }
    assert(WritableBytes() >= len);
}

//已经有len个字节的数据被写入到缓冲区中。
void Buffer::HasWritten(size_t len)
{
    _writePos += len;
}

//将读取位置向后移动len个字节
void Buffer::Retrieve(size_t len)
{
    assert(len <= ReadableBytes());
    _readPos += len;
}

//从当前读取位置开始，一直读取到指针end所指的位置，
void Buffer::RetrieveUntil(const char* end)
{
    assert(Peek() <= end);
    Retrieve(end - Peek());
}

//清空缓冲区中的所有数据，将读取位置和写入位置重置为0
void Buffer::RetrieveAll()
{
    bzero(&_buffer[0], _buffer.size());
    _readPos = 0;
    _writePos = 0;
}


//从缓冲区中获取所有可读数据，并将其转换为字符串。
std::string Buffer::RetrieveAllToStr()
{
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}


// 返回一个指向缓冲区中可写数据位置的常量指针
const char* Buffer::BeginWriteConst() const
{
    return _BeginPtr() + _writePos;
}

// 返回一个指向缓冲区中可写数据位置的指针。
char* Buffer::BeginWrite()
{
    return _BeginPtr() + _writePos;
}

//追加一个str到缓冲区
void Buffer::Append(const std::string& str)
{
    Append(str.data(), str.length());
}

void Buffer::Append(const char* str, size_t len)
{
    assert(str);
    EnsureWritable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const void* data, size_t len)
{
    assert(data);
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const Buffer& buff)
{
    Append(buff.Peek(), buff.ReadableBytes());
}

//从给定的文件描述符(fd)中读取数据，并将其存储到Buffer对象中。
ssize_t Buffer::ReadFd(int fd, int* Errno)
{
    char buff[65536];//在存数据
    struct iovec iov[2];//用于指定读取数据的内存地址和长度。
    const size_t writable = WritableBytes();
    // 分散读，保证数据全读完
    //iov数组包含两个元素，分别指向
    //Buffer对象中未写入部分的内存地址和长度
    //以及buff数组的内存地址和长度。
    iov[0].iov_base = _BeginPtr() + _writePos;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    // 调用readv函数，从文件描述符fd中读取数据。readv函数会将数据分散读取到iov数组指定的内存地址中。
    const ssize_t len = readv(fd, iov, 2);

    //读取失败
    if(len < 0){
        *Errno = errno;
    }
    // 全部数据在第一个缓冲区内
    else if (static_cast<size_t>(len) <= writable) {
        _writePos += len;
    }
    // 部分数据在第二个缓冲区
    else{
        _writePos = _buffer.size();
        Append(buff, len - writable);
    }
    return len;
}

// 将Buffer对象中的数据写入给定的文件描述符(fd)。
ssize_t Buffer::WriteFd(int fd, int* Errno)
{
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);

    if(len < 0){
        *Errno = errno;
        return len;
    }
    _readPos += len;
    return len;
}

//返回起始位置
char* Buffer::_BeginPtr()
{
    return &*_buffer.begin();
}

const char* Buffer::_BeginPtr() const
{
    return &*_buffer.begin();
}
// |---------_readPos(PrependableBytes)--------_writePos------------|end
void Buffer::_MakeSpace(size_t len)
{   
    //目前可写的位置和已读的位置加起来都不够需要的空间
    if(WritableBytes() + PrependableBytes() < len){
        _buffer.resize(_writePos + len + 1);
    }
    //可写的位置和已读的位置加起来够空间，不需要开辟空间
    else{
        size_t readable = ReadableBytes();
        std::copy(_BeginPtr() + _readPos, _BeginPtr() + _writePos, _BeginPtr());
        _readPos = 0;
        _writePos = _readPos + readable;
        assert(readable == ReadableBytes());
    }
}




