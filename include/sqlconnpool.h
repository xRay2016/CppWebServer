/*
 * @Author: your name
 * @Date: 2021-09-02 23:00:45
 * @LastEditTime: 2021-09-02 23:34:47
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/code/pool/sqlconnpool.h
 */
#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include <assert.h>

#include "log.h"

class SqlConnPool{
public:
    static SqlConnPool *Instance();

    MYSQL *GetConn();

    void FreeConn(MYSQL * conn);
    int GetFreeConnCount();

    int Init(const char* hots,int port,
            const char* user,const char* pwd,
            const char* dbName,int connSize=10);
    void ClosePool();
private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_;
    int useCount_;
    int freeCount_;

    std::queue<MYSQL*> _connQue;
    std::mutex _mtx;
    sem_t _semId;
};

#endif
