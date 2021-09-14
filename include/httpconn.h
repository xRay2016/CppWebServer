/*
 * @Author: your name
 * @Date: 2021-09-08 23:23:20
 * @LastEditTime: 2021-09-12 21:16:23
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/include/httpconn.h
 */
#ifndef HTTPCONN_H
#define HTTPCONN_H

#include <sys/types.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>

#include "log.h"
#include "sqlConnRAII.hpp"
#include "buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn{
public:
    HttpConn();
    ~HttpConn();

    void init(int sockfd,const sockaddr_in& addr);

    ssize_t read(int* saveErrno);

    ssize_t write(int* saveErrno);

    void close_();

    int getFd() const;
    int getPort() const;
    const char* getIp() const;
    sockaddr_in getAddr() const;

    bool process();
    int ToWriteBytes(){
        return iov_[0].iov_len+iov_[1].iov_len;
    }

    bool isKeepAlive() const{
        return request_.isKeepAlive();
    }

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;

private:

    
private:
    int fd_;
    sockaddr_in addr_;
    bool isClose_;
    int iovCnt_;
    iovec iov_[2];

    Buffer readBuff_;
    Buffer writeBuff_;

    HttpRequest request_;
    HttpResponse response_;
};

#endif