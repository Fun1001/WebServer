/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-07-05 17:08:18
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-07-05 17:25:29
 * @FilePath: /WebServer/server/epoller.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "epoller.h"
#include <cassert>
#include <sys/epoll.h>
#include <unistd.h>

//创建一个epoll实例
Epoller::Epoller(int maxEvent): _epollFd(epoll_create(512)), _events(maxEvent)
{
    assert(_epollFd >= 0 && _events.size() > 0)
}

Epoller::~Epoller()
{
    close(_epollFd);
}

bool Epoller::AddFd(int fd, uint32_t events)
{
    if (fd < 0) {
        return false;
    }
    // 创建一个epoll_event结构体实例，初始化为0
    epoll_event ev = {0};
    // 设置文件描述符到结构体的data.fd成员
    ev.data.fd = fd;
    // 设置要监控的事件类型到结构体的events成员
    ev.events = events;
    // 尝试将文件描述符添加到epoll实例
    // _epollFd是Epoller类中存储epoll实例文件描述符的成员变量
    // EPOLL_CTL_ADD是操作类型，表示添加操作
    // 返回值是epoll_ctl函数的执行结果
    return 0 == epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::ModFd(int fd, uint32_t events)
{
    if (fd < 0) {
        return false;
    }
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;

    return 0 == epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::DelFd(int fd)
{
    if (fd < 0) {
        return false;
    }
    epoll_event ev = {0};

    return 0 == epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::Wait(int timeoutMs)
{
    return epoll_wait(_epollFd, &_events[0], static_cast<int>(_events.size()), timeoutMs);
}

int Epoller::GetEventFd(size_t i) const
{
    assert(i < _events.size() && i >= 0);
    return _events[i].data.fd;
}

uint32_t Epoller::GetEvents(size_t i) const
{
    assert(i < _events.size() && i >= 0);
    return _events[i].events;
}
