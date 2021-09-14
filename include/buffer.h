/*
 * @Author: your name
 * @Date: 2021-08-30 20:45:48
 * @LastEditTime: 2021-09-13 20:34:39
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/code/buffer/Buffer.h
 */
#ifndef BUFFER_H
#define BUFFER_H

#include <string.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <atomic>
#include <assert.h>
#include <sys/uio.h>

class Buffer{
public:
    Buffer(int initBufferSize=1024);
    ~Buffer()=default;

    /**
     * @description: 可写的缓冲区比特数 
     * @param {*}
     * @return {*} 返回可写的缓冲区比特数
     */    
    size_t WritableBytes() const;

    /**
     * @description:  可读的缓冲区比特数
     * @param {*}
     * @return {*}  返回可读的缓冲区比特数
     */    
    size_t ReadableBytes() const;

    /**
     * @description: 返回读位置readPos
     */    
    size_t PrependableBytes() const;

    /**
     * @description: 返回读位置的指针
     * @param {*}
     * @return {*} 返回读位置的指针
     */
    const char* Peek() const;

    /**
     * @description: 
     * @param {size_t} len
     * @return {*}
     */    
    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);

    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);
    void RetrieveAll();
    std::string RetrieveAllToStr();

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const std::string& str);
    void Append(const char* str,size_t len);
    void Append(const void* data,size_t len);
    void Append(const Buffer& buff);

    ssize_t ReadFd(int fd,int* Errno);
    ssize_t WriteFd(int fd,int* Errno);


private:
    char* BeginPtr_();
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len);
    
    /* 字符数组 */
    std::vector<char> buffer_;
    
    /* 读位置 */
    std::atomic<size_t> readPos_;
    
    /* 写位置 */
    std::atomic<size_t> writePos_;
};


#endif
