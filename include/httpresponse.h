/*
 * @Author: your name
 * @Date: 2021-09-10 10:42:30
 * @LastEditTime: 2021-09-12 21:28:46
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/include/httpresponse.h
 */
#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "buffer.h"
#include "log.h"

class HttpResponse{
public:
    HttpResponse();
    ~HttpResponse();

    void init(const std::string& srcDir,const std::string& path,bool isKeepAlive=false,int code=-1);
    void makeResponse(Buffer& buff);
    void unmapFile();
    char* File();
    size_t Filelen() const;
    void ErrorConetent(Buffer& buff,std::string message);
    int code() const{return code_;}

private:
    int code_;
    bool isKeepAlive_;

    std::string path_;
    std::string srcDir_;

    char* mmFile_;
    struct stat mmFileStat_;

    static const std::unordered_map<std::string,std::string> SUFFIX_TYPE;
    static const std::unordered_map<int,std::string> CODE_STATUS;
    static const std::unordered_map<int,std::string> CODE_PATH;

private:
    void AddStateLine_(Buffer& buff);
    void AddHeader_(Buffer& buff);
    void AddContent_(Buffer& buff);
    void ErrorHtml_();
    std::string GetFileType_();
};

#endif