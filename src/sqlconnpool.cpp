/*
 * @Author: your name
 * @Date: 2021-09-02 23:15:56
 * @LastEditTime: 2021-09-08 11:06:00
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/src/sqlconnpool.cpp
 */

#include "sqlconnpool.h"
using namespace std;

SqlConnPool::SqlConnPool()
{
    useCount_ = 0;
    freeCount_ = 0;
}

SqlConnPool *SqlConnPool::Instance()
{
    static SqlConnPool connPool;
    return &connPool;
}

int SqlConnPool::Init(const char *host, int port,
                      const char *user, const char *pwd,
                      const char *dbName, int connSize)
{
    assert(connSize > 0);
    for (int i = 0; i < connSize; i++)
    {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if (!sql)
        {
            LOG_ERROR("Mysql init error!");
            assert(sql);
        }
        sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
        if(!sql)
        {
            LOG_ERROR("MySql Connect error!");
        }
        _connQue.push(sql);
    }
    MAX_CONN_=connSize;
    sem_init(&_semId,0,MAX_CONN_);
}

MYSQL* SqlConnPool::GetConn()
{
    MYSQL* sql=nullptr;
    if(_connQue.empty()){
        LOG_WARN("SqlConnPool busy!");
        return nullptr;
    }
    sem_wait(&_semId);

    {
        lock_guard<mutex> locker(_mtx);
        sql=_connQue.front();
        _connQue.pop();
    }
    return sql;
}

void SqlConnPool::FreeConn(MYSQL* sql)
{
    assert(sql);
    lock_guard<mutex> locker(_mtx);
    _connQue.push(sql);
    sem_post(&_semId);
}

void SqlConnPool::ClosePool()
{
    lock_guard<mutex> locker(_mtx);
    while(!_connQue.empty()){
        auto item=_connQue.front();
        _connQue.pop();
        mysql_close(item);
    }
    mysql_library_end();
}

int SqlConnPool::GetFreeConnCount()
{
    lock_guard<mutex> locker(_mtx);
    return _connQue.size();
}

SqlConnPool::~SqlConnPool()
{
    ClosePool();
}
