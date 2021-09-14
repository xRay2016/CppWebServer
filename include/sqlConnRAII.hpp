/*
 * @Author: your name
 * @Date: 2021-09-08 11:07:09
 * @LastEditTime: 2021-09-08 11:32:51
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/include/sqlConnRAII.hpp
 */
#ifndef SQLCONNRAII_H
#define SQLCONNRAII_H
#include "sqlconnpool.h"

class SqlConnRAII{
public:
    SqlConnRAII(MYSQL*& sql,SqlConnPool* connpool){
        assert(_connpool);
        sql=connpool->GetConn();
        _sql=sql;
        _connpool=connpool;
    }

    ~SqlConnRAII(){
        if(_sql){_connpool->FreeConn(_sql);}
        _sql=nullptr;
    }

private:
    MYSQL* _sql;
    SqlConnPool* _connpool;
};

#endif