/*
 * @Author: your name
 * @Date: 2021-08-30 21:56:37
 * @LastEditTime: 2021-10-07 22:29:57
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/main.cpp
 */
#include <unistd.h>
#include "webserver.h"

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

void server_process()
{
    WebServer server_instance(
        3697,ALL_ET,60000,false,
        3306,"root","980825","webserver",
        12,6,true,1,1024,
        "/root/source_code/cpp_server/log"
    );
    server_instance.start();
}

int main()
{
    server_process();
}