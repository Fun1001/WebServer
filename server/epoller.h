/*
 * @Author: Jiuchuan jiuchuanfun@gmail.com
 * @Date: 2024-07-05 17:08:24
 * @LastEditors: Jiuchuan jiuchuanfun@gmail.com
 * @LastEditTime: 2024-07-05 17:15:26
 * @FilePath: /WebServer/server/epoller.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __EPOLLER_H__
#define __EPOLLER_H__

#include <cstdint>
#include <sys/epoll.h> //epoll_ctl()
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <assert.h> // close()
#include <vector>
#include <errno.h>

class Epoller {
public:
    explicit Epoller(int maxEvent = 1024);

    ~Epoller();

    bool AddFd(int fd, uint32_t events);

    bool ModFd(int fd, uint32_t events);

    bool DelFd(int fd);

    int Wait(int timeoutMs = -1);

    int GetEventFd(size_t i) const;

    uint32_t GetEvents(size_t i) const;

private:
    int _epollFd;

    std::vector<struct epoll_event> _events;
};

#endif // __EPOLLER_H__