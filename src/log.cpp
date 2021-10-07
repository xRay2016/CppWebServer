/*
 * @Author: your name
 * @Date: 2021-09-02 23:33:09
 * @LastEditTime: 2021-10-06 21:33:32
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/src/log.cpp
 */
#include "log.h"

using namespace std;

Log::Log() : _linecount(0), _isAsync(false), _writeThread(nullptr),
             _today(0),_deque(nullptr)
{
}

Log::~Log()
{
    if(_writeThread&&_writeThread->joinable()){
        while(!_deque->empty()){
            _deque->flush();
        }
        _deque->close();
        _writeThread->join();
    }
    if(_fp){
        lock_guard<mutex> locker(_mtx);
        flush();
        fclose(_fp);
    }
}

int Log::GetLevel(){
    lock_guard<mutex> locker(_mtx);
    return _level;
}

void Log::SetLevel(int level){
    lock_guard<mutex> locker(_mtx);
    _level=level;
}

void Log::init(int level=1,const char* path,const char* suffix,int maxQueueSize)
{
    _isOpen=true;
    _level=level;
    if(maxQueueSize>0){
        _isAsync=true;
        if(!_deque){
            //unique_ptr<BlockQueue<std::string>> newQueue(new BlockQueue<std::string>);
            _deque=make_unique<BlockQueue<std::string>>();
            //unique_ptr<thread> newThread(new thread(FlushLogThread));
            _writeThread=make_unique<std::thread>(FlushLogThread);
        }
    }
    else{
        _isAsync=false;
    }
    _linecount=0;

    time_t timer=time(nullptr);
    tm t=*localtime(&timer);

    _path=path;
    _suffix=suffix;
    char fileName[LOG_NAME_LEN]={0};
    snprintf(fileName,LOG_NAME_LEN-1,"%s/%04d_%02d_%02d%s",
            _path,t.tm_year+1900,t.tm_mon+1,t.tm_mday,_suffix);
    
    _today=t.tm_mday;

    {
        lock_guard<mutex> locker(_mtx);
        _buffer.RetrieveAll();
        if(_fp){
            flush();
            fclose(_fp);
        }
        _fp=fopen(fileName,"a");
        if(_fp==nullptr){
            mkdir(_path,0777);
            _fp=fopen(fileName,"a");
        }
        assert(_fp!=nullptr);
    }
}


void Log::write(int level,const char* format,...)
{
    timeval now={0,0};
    gettimeofday(&now,nullptr);
    time_t tSec=now.tv_sec;
    tm t =*localtime(&tSec);

    va_list vaList;
    if(_today!=t.tm_mday||(_linecount && _linecount % MAX_LINES == 0))
    {
        unique_lock<mutex> locker(_mtx);
        locker.unlock();
        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if (_today != t.tm_mday)
        {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", _path, tail, _suffix);
            _today = t.tm_mday;
            _linecount = 0;
        }
        else {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", _path, tail, (_linecount  / MAX_LINES), _suffix);
        }
        
        locker.lock();
        flush();
        fclose(_fp);
        _fp = fopen(newFile, "a");
        assert(_fp != nullptr);
    }

    {
        unique_lock<mutex> locker(_mtx);
        _linecount++;
        int n = snprintf(_buffer.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
                    
        _buffer.HasWritten(n);
        AppendLogLevelTitle(level);

        va_start(vaList, format);
        int m = vsnprintf(_buffer.BeginWrite(), _buffer.WritableBytes(), format, vaList);
        va_end(vaList);

        _buffer.HasWritten(m);
        _buffer.Append("\n\0", 2);

        if(_isAsync && _deque && !_deque->full()) {
            string info=_buffer.RetrieveAllToStr();
            //cout<<info<<endl;
            _deque->push_back(info);
        } else {
            fputs(_buffer.Peek(), _fp);
        }
        flush();
        _buffer.RetrieveAll();
    }
}

void Log::AppendLogLevelTitle(int level) {
    switch(level) {
    case 0:
        _buffer.Append("[debug]: ", 9);
        break;
    case 1:
        _buffer.Append("[info] : ", 9);
        break;
    case 2:
        _buffer.Append("[warn] : ", 9);
        break;
    case 3:
        _buffer.Append("[error]: ", 9);
        break;
    default:
        _buffer.Append("[info] : ", 9);
        break;
    }
}

void Log::flush() {
    if(_isAsync) { 
        _deque->flush(); 
    }
    fflush(_fp);
}

void Log::AsyncWrite() {
    string str = "";
    while(_deque->pop(str)) {
        lock_guard<mutex> locker(_mtx);
        fputs(str.c_str(), _fp);
    }
}

Log* Log::Instance() {
    static Log inst;
    return &inst;
}

void Log::FlushLogThread() {
    Log::Instance()->AsyncWrite();
}






