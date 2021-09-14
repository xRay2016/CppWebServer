/*
 * @Author: your name
 * @Date: 2021-09-09 14:51:14
 * @LastEditTime: 2021-09-10 10:41:01
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/include/httprequest.h
 */
#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>
#include <mysql/mysql.h>

#include "buffer.h"
#include "log.h"
#include "sqlconnpool.h"
#include "sqlConnRAII.hpp"

class HttpRequest{
public:
    enum PARSE_STATE{
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

    enum HTTP_CODE{
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NP_RESOURCE,
    };

    HttpRequest(){init();};
    ~HttpRequest()=default;

    void init();
    bool parse(Buffer& buff);

    std::string path() const;
    std::string method()const;
    std::string version()const;
    std::string getPost(const std::string& key)const;
    std::string getPost(const char* key)const;
    
    bool isKeepAlive() const;
    
    
private:
    PARSE_STATE state_;
    std::string method_,path_,version_,body_;
    std::unordered_map<std::string,std::string> header_;
    std::unordered_map<std::string,std::string> post_;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string,int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch);

private:
    bool parseRequestLine_(const std::string& line);
    void parseHeader_(const std::string& line);
    void parseBody_(const std::string& line);

    void parsePath_();
    void parsePost_();
    void parseFromcoded_();

    static bool UserVerify(const std::string& name,const std::string& pwd,bool isLogin);
    
};

#endif