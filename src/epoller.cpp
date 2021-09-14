/*
 * @Author: your name
 * @Date: 2021-09-08 22:37:25
 * @LastEditTime: 2021-09-08 22:51:50
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/src/Epoller.cpp
 */
#include "epoller.h"

Epoller::Epoller(int maxEvent):_events(maxEvent)
{
    _epollfd=epoll_create(maxEvent);
    assert(_epollfd>=0 && _events.size()>0);
}

Epoller::~Epoller()
{
    close(_epollfd);
}

bool Epoller::addFd(int fd,uint32_t events)
{
    if(fd<0)
        return false;
    epoll_event ev={0};
    ev.events=events;
    ev.data.fd=fd;
    return epoll_ctl(_epollfd,EPOLL_CTL_ADD,fd,&ev)==0;
}

bool Epoller::modFd(int fd,uint32_t events)
{
    if(fd<0)
        return false;
    epoll_event ev={0};
    ev.events=events;
    ev.data.fd=fd;
    return epoll_ctl(_epollfd,EPOLL_CTL_MOD,fd,&ev);
}

bool Epoller::DelFd(int fd)
{
    if(fd<0)
        return false;
    return epoll_ctl(_epollfd,EPOLL_CTL_DEL,fd,nullptr)==0;
}

int Epoller::wait(int timeoutMs)
{
    return epoll_wait(_epollfd,&_events[0],static_cast<int>(_events.size()),timeoutMs);
}

int Epoller::getEventFd(size_t i) const
{
    assert(i<_events.size() && i>=0);
    return _events[i].data.fd;
}

uint32_t Epoller::getEvents(size_t i) const
{
    assert(i<_events.size()&&i>=0);
    return _events[i].events;
}





