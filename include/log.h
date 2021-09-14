/*
 * @Author: your name
 * @Date: 2021-09-02 23:33:04
 * @LastEditTime: 2021-09-07 23:30:55
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/include/log.h
 */
#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "blockqueue.hpp"
#include "buffer.h"

class Log
{
public:
    void init(int level, const char *path = "./log", const char *suffix = ".log",int maxQueueCount=1024);

    static Log* Instance();

    static void FlushLogThread();

    void write(int level,const char* fromat,...);
    void flush();
    int GetLevel();
    void SetLevel(int level);
    bool IsOpen(){return _isOpen;}

private:
    Log();
    virtual ~Log();
    void AppendLogLevelTitle(int level);
    void AsyncWrite();

private:
    static const int LOG_PATH_LEN=256;
    static const int LOG_NAME_LEN=512;
    static const int MAX_LINES=50000;

    const char* _path;
    const char* _suffix;

    int _MAX_LINES;

    int _linecount;
    int _today;
    bool _isOpen;

    Buffer _buffer;
    int _level;
    bool _isAsync;

    FILE* _fp;
    std::unique_ptr<std::thread> _writeThread;
    std::unique_ptr<BlockQueue<std::string>> _deque;
    std::mutex _mtx;

};

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::Instance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);


#endif