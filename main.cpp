/*
 * @Author: your name
 * @Date: 2021-08-30 21:56:37
 * @LastEditTime: 2021-09-14 15:00:53
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/main.cpp
 */
#include <iostream>                // std::cout
#include <thread>                // std::thread
#include <mutex>                // std::mutex, std::unique_lock
#include <condition_variable>    // std::condition_variable
#include <vector>
#include <ctime>
#include <chrono>
#include <string.h>
#include <sys/mman.h>
#include "heaptimer.h"
#include "log.h"
#include "httprequest.h"
#include "httpconn.h"

using namespace std;
 
void test_http()
{
    string request="";
    HttpConn conn;
    sockaddr_in addr={0};
    addr.sin_family=AF_INET;
    addr.sin_port=htons(3098);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    int sock_fd=open("test_http",O_RDONLY);
    conn.init(sock_fd,addr);
    int saveErrno;
    ssize_t len=conn.read(&saveErrno);
    HttpConn::srcDir="./resources/";
    conn.process();
}

int main()
{
    try{
        regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    }
    catch(const regex_error& e){
        cout<<regex_constants::error_brack<<endl;
        cout<<e.code()<<endl;
    }
    test_http();
}