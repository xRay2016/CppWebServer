<!--
 * @Author: your name
 * @Date: 2021-10-07 11:07:04
 * @LastEditTime: 2021-10-07 22:09:22
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/README.md
-->

# CppWebServer

```
  ____          __        __   _    ____                           
 / ___|_ __  _ _\ \      / /__| |__/ ___|  ___ _ ____   _____ _ __ 
| |   | '_ \| '_ \ \ /\ / / _ \ '_ \___ \ / _ \ '__\ \ / / _ \ '__|
| |___| |_) | |_) \ V  V /  __/ |_) |__) |  __/ |   \ V /  __/ |   
 \____| .__/| .__/ \_/\_/ \___|_.__/____/ \___|_|    \_/ \___|_|   
      |_|   |_|                                                    
```

利用C++实现的高性能Web服务器，经过webbench的压力测试可以实现万级的QPS。

## 功能
* 利用IO复用技术Epoll和线程池实现多线程的Reactor并发模型
* 基于小顶堆实现的定时器，关闭不活跃的链接
* 单例模式和阻塞队列实现的异步日志系统，记录服务器的运行状态
* 实现RAII机制的数据库连接池，减少了数据库连接建立和关闭的开销
* 基于正则和状态机解析HTTP请求报文，实现了静态资源的请求

## 环境要求

* Linux
* Mysql

## 项目启动
首先需要配置好用户的数据库

```mysql
//建立数据库
create database webserver;

//建立user表
use webserver;
create table user(
  username char(50) NULL,
  password char(50) NULL
)ENGINE=InnoDB;

//添加用户数据
insert into user(username,password) VALUES('name', 'password');

```

```bash
mkdir build && cd build
cmake ..
make -j4
../web_server
```

## 压力测试


```
webbench -c 100 -t 10 http://ip:port/
webbench -c 1000 -t 10 http://ip:port/
webbench -c 5000 -t 10 http://ip:port/
webbench -c 10000 -t 10 http://ip:port/
```