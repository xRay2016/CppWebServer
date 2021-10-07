/*
 * @Author: your name
 * @Date: 2021-09-08 22:52:32
 * @LastEditTime: 2021-09-14 20:13:13
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/include/webserver.h
 */
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"
#include "log.h"
#include "heaptimer.h"
#include "sqlconnpool.h"
#include "threadpool.h"
#include "sqlConnRAII.hpp"
#include "httpconn.h"

enum TRIGMODE{
    ALL_LT,ALL_ET,LISTEN_ET,CONNECT_ET
};

class WebServer{
public:
    WebServer(
        int port,TRIGMODE trigMode,int timeoutMS,bool OptLinger,
        int sqlPort,const char* sqlUser,const char* sqlPwd,
        const char* dbName, int connPoolNum,int threadNum,
        bool openLog,int logLevel,int logQueSize,const char* logPath);

    ~WebServer();

    void start();
private:
    int _port;
    bool _openLinger;
    int _timeoutMS;
    bool _isClose;
    int _listenFd;
    char* srcDir;

    uint32_t _listenEvent;
    uint32_t _connEvent;

    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int,HttpConn> user_;

private:
    bool initSocket_();
    void initEventMode_(TRIGMODE trigMode);
    void addClient_(int fd, sockaddr_in addr);
    
    void deaListen_();
    void dealWrite_(HttpConn* client);
    void dealRead_(HttpConn* client);

    void sendError_(int fd,const char* info);
    void extentTime_(HttpConn* client);
    void closeConn_(HttpConn* client);

    void onRead_(HttpConn* client);
    void onWrite_(HttpConn* client);
    void onProcess_(HttpConn* client);
    
    static const int MAX_FD=65536;

    static int setFdNonblock(int fd);
};

#endif