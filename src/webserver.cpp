/*
 * @Author: your name
 * @Date: 2021-09-08 22:52:39
 * @LastEditTime: 2021-10-07 23:18:32
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/src/webserver.cpp
 */
#include "webserver.h"

WebServer::WebServer(
    int port, TRIGMODE trigMode, int timeoutMS, bool OptLinger,
    int sqlPort, const char* sqlUser, const  char* sqlPwd,
    const char* dbName, int connPoolNum, int threadNum,
    bool openLog, int logLevel, int logQueSize, const char* logPath):
    _port(port),_openLinger(OptLinger),_timeoutMS(timeoutMS),_isClose(false),
    timer_(new HeapTimer()),threadPool_(new ThreadPool(threadNum)),epoller_(new Epoller())
{
    srcDir=getcwd(nullptr,256);
    assert(srcDir);
    strncat(srcDir,"/resources",16);
    HttpConn::userCount=0;
    HttpConn::srcDir=srcDir;

    SqlConnPool::Instance()->Init("localhost",sqlPort,sqlUser,sqlPwd,dbName,connPoolNum);

    initEventMode_(trigMode);
    if(!initSocket_()){
        _isClose=true;
    }
    if(openLog){
        Log::Instance()->init(logLevel,logPath,".log",logQueSize);
        if(_isClose){
            LOG_ERROR("========== Server init error!==========");
        }
        else{
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", _port, OptLinger? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (_listenEvent & EPOLLET ? "ET": "LT"),
                            (_connEvent & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}

WebServer::~WebServer()
{
    close(_listenFd);
    _isClose=true;
    free(srcDir);
    SqlConnPool::Instance()->ClosePool();
}

void WebServer::initEventMode_(TRIGMODE trigmode)
{
    _listenEvent=EPOLLRDHUP;
    _connEvent=EPOLLONESHOT | EPOLLRDHUP;
    switch(trigmode)
    {
    case ALL_LT:
        break;
    case ALL_ET:
        _listenEvent |= EPOLLET;
        _connEvent |= EPOLLET;
        break;
    case LISTEN_ET: 
        _listenEvent |= EPOLLET;
        break;
    case CONNECT_ET:
        _connEvent |= EPOLLET;
        break;
    default:
        _listenEvent |= EPOLLET;
        _connEvent |= EPOLLET;
        break;  
    }
    HttpConn::isET=(_connEvent&EPOLLET);
}

void WebServer::start()
{
    volatile int timeMs=-1;
    if(!_isClose)
    {
        LOG_INFO("========== Server start ==========");
        while(!_isClose){
            if(_timeoutMS>0){
                //printf("hello world!\n");
                timeMs=timer_->GetNextTick();
            }
            int eventCnt =epoller_->wait(timeMs);
            for(int i=0;i<eventCnt;i++){
                int fd=epoller_->getEventFd(i);
                uint32_t events=epoller_->getEvents(i);
                if(fd==_listenFd){
                    deaListen_();
                }
                else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                    assert(user_.count(fd)>0);
                    //关闭fd上的连接
                    closeConn_(&user_[fd]);
                }
                else if(events & EPOLLIN){
                    assert(user_.count(fd)>0);
                    //处理读
                    dealRead_(&user_[fd]);
                }
                else if(events & EPOLLOUT){
                    assert(user_.count(fd)>0);
                    //处理写
                    dealWrite_(&user_[fd]);
                }
                else{
                    LOG_ERROR("Unexpected event");
                }
            }
        }
    }
}

void WebServer::sendError_(int fd, const char*info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

void WebServer::deaListen_()
{
    sockaddr_in addr;
    socklen_t len=sizeof(addr);
    do{
        int fd=accept(_listenFd,(sockaddr*)&addr,&len);
        if(fd<=0){
            return;
        }
        else if(HttpConn::userCount>=MAX_FD){
            sendError_(fd,"server busy");
            LOG_WARN("Clients is full");
            return;
        }
        addClient_(fd,addr);
    }while(_listenEvent & EPOLLET);
}

void WebServer::addClient_(int fd,sockaddr_in addr)
{
    assert(fd>0);
    user_[fd].init(fd,addr);
    if(_timeoutMS>0){
        timer_->add(fd,_timeoutMS,std::bind(&WebServer::closeConn_,this,&user_[fd]));
    }
    epoller_->addFd(fd,_connEvent | EPOLLIN);
    setFdNonblock(fd);
    LOG_INFO("Client[%d] in!", user_[fd].getFd());
}

void WebServer::closeConn_(HttpConn* client)
{
    assert(client);
    LOG_INFO("Client[%d] quit!", client->getFd());
    epoller_->DelFd(client->getFd());
    client->close_();
}

void WebServer::dealRead_(HttpConn* client)
{
    assert(client);
    extentTime_(client);
    threadPool_->AddTask(std::bind(&WebServer::onRead_,this,client));
}

void WebServer::onRead_(HttpConn* client)
{
    assert(client);
    int ret=-1;
    int readErrno=0;
    ret=client->read(&readErrno);
    if(ret<=0&&readErrno!=EAGAIN)
    {
        closeConn_(client);
        return;
    }
    onProcess_(client);
}

void WebServer::onWrite_(HttpConn* client)
{
    assert(client);
    int ret=-1;
    int writeErrno=0;
    ret=client->write(&writeErrno);
    if(client->ToWriteBytes()==0){
        if(client->isKeepAlive()){
            onProcess_(client);
            return;
        }
    }
    else if(ret<0){
        if(writeErrno==EAGAIN){
            epoller_->modFd(client->getFd(),_connEvent|EPOLLOUT);
            return;
        }
    }
    closeConn_(client);
}

void WebServer::dealWrite_(HttpConn* client)
{
    assert(client);
    extentTime_(client);
    threadPool_->AddTask(std::bind(&WebServer::onWrite_,this,client));
}

void WebServer::extentTime_(HttpConn* client)
{
    assert(client);
    if(_timeoutMS>0){
        timer_->adjust(client->getFd(),_timeoutMS);
    }
}

void WebServer::onProcess_(HttpConn* client)
{
    assert(client);
    if(client->process()){
        epoller_->modFd(client->getFd(),_connEvent|EPOLLOUT);
    }
    else{
        epoller_->modFd(client->getFd(),_connEvent|EPOLLIN);
    }
}

bool WebServer::initSocket_(){
    int ret=0;
    sockaddr_in addr;
    if(_port>65535 || _port <1024)
    {
        LOG_ERROR("Port:%d error!",_port);
        return false;
    }
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(_port);
    linger optLinger={0};
    if(_openLinger){
        optLinger.l_onoff=1;
        optLinger.l_linger=1;
    }

    _listenFd=socket(AF_INET,SOCK_STREAM,0);
    if(_listenFd<0){
        LOG_ERROR("Create socket error!", _port);
        return false;
    }

    ret=setsockopt(_listenFd,SOL_SOCKET,SO_LINGER,&optLinger,sizeof(optLinger));
    if(ret<0){
        close(_listenFd);
        LOG_ERROR("Init linger error!",_port);
        return false;
    }

    int optval=1; 
    ret=setsockopt(_listenFd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
    if(ret == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(_listenFd);
        return false;
    }

    ret=bind(_listenFd,(sockaddr*)&addr,sizeof(addr));
    if(ret<0){
        LOG_ERROR("Bind Port%d error!",_port);
        close(_listenFd);
        return false;
    }

    ret=listen(_listenFd,6);
    if(ret<0){
        LOG_ERROR("Listen port error!",_port);
        close(_listenFd);
        return false;
    }

    ret=epoller_->addFd(_listenFd,_listenEvent|EPOLLIN);
    if(ret==0){
        LOG_ERROR("Add listen error");
        close(_listenFd);
        return false;
    }
    setFdNonblock(_listenFd);
    LOG_INFO("Server port:%d",_port);
    return true;
}

int WebServer::setFdNonblock(int fd)
{
    assert(fd>0);
    int old_option=fcntl(fd,F_GETFD,0);
    fcntl(fd,F_SETFL,old_option|O_NONBLOCK);
    return old_option;
}