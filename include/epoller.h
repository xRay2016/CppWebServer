/*
 * @Author: your name
 * @Date: 2021-09-08 22:32:31
 * @LastEditTime: 2021-09-08 22:50:44
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/include/epoller.h
 */
#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <errno.h>

class Epoller{
public:
    explicit Epoller(int maxEvent=1024);

    ~Epoller();

    bool addFd(int fd,uint32_t events);

    bool modFd(int fd,uint32_t events);

    bool DelFd(int fd);

    int wait(int timoutMs=-1);

    int getEventFd(size_t i) const;

    uint32_t getEvents(size_t i) const;

private:
    int _epollfd;

    std::vector<epoll_event> _events;
};

#endif
