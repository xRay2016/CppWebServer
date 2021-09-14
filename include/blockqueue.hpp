/*
 * @Author: your name
 * @Date: 2021-09-03 12:49:41
 * @LastEditTime: 2021-09-07 23:23:14
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/include/blockqueue.h
 */
#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>
#include <assert.h>

template <class T>
class BlockQueue
{
public:
    explicit BlockQueue(size_t maxCapcity = 1000);

    ~BlockQueue();

    void clear();

    bool empty();

    bool full();

    void close();

    size_t size();

    size_t capcity();

    T front();

    T back();

    void push_back(const T &item);

    void push_front(const T &item);

    bool pop(T &item);

    bool pop(T &item, int timeout);

    void flush();

private:
    std::deque<T> _queue;
    size_t _capcity;
    std::mutex _mtx;
    bool _isClose;
    std::condition_variable _cond_producer;
    std::condition_variable _cond_consumer;
};

template <class T>
BlockQueue<T>::BlockQueue(size_t capcity) : _capcity(capcity),
                                            _isClose(false)
{
    assert(capcity > 0);
}

template<class T>
BlockQueue<T>::~BlockQueue()
{
    close();
}

template<class T>
void BlockQueue<T>::clear()
{
    std::lock_guard<std::mutex> locker(_mtx);
    _queue.clear();
}

template<class T>
void BlockQueue<T>::close()
{
    {
        std::lock_guard<std::mutex> locker(_mtx);
        _queue.clear();
        _isClose=true;
    }
    _cond_consumer.notify_all();
    _cond_producer.notify_all();
}

template<class T>
void BlockQueue<T>::flush()
{
    _cond_consumer.notify_one();
}

template<class T>
bool BlockQueue<T>::empty()
{
    std::lock_guard<std::mutex> locker(_mtx);
    return _queue.empty();
}

template<class T>
bool BlockQueue<T>::full()
{
    std::lock_guard<std::mutex> locker(_mtx);
    return _queue.size()>=_capcity;
}

template<class T>
size_t BlockQueue<T>::size()
{
    std::lock_guard<std::mutex> locker(_mtx);
    return _queue.size();
}

template<class T>
size_t BlockQueue<T>::capcity()
{
    std::lock_guard<std::mutex> locker(_mtx);
    return _capcity;
}

template<class T>
T BlockQueue<T>::front()
{
    std::lock_guard<std::mutex> locker(_mtx);
    return _queue.front();
}

template<class T>
T BlockQueue<T>::back()
{
    std::lock_guard<std::mutex> locker(_mtx);
    return _queue.back();
}

template<class T>
void BlockQueue<T>::push_back(const T& item)
{   
    std::unique_lock<std::mutex> locker(_mtx);
    while(_queue.size()>=_capcity){
        _cond_producer.wait(locker);
    }
    _queue.push_back(item);
    _cond_consumer.notify_one();
}

template<class T>
void BlockQueue<T>::push_front(const T& item)
{
    std::unique_lock<std::mutex> locker(_mtx);
    while(_queue.size()>=_capcity){
        _cond_producer.wait(locker);
    }
    _queue.push_front(item);
    _cond_consumer.notify_one();
}

template<class T>
bool BlockQueue<T>::pop(T& item)
{
    std::unique_lock<std::mutex> locker(_mtx);
    while(_queue.empty()){
        _cond_consumer.wait(locker);
        if(_isClose){
            return false;
        }
    }
    item=_queue.front();
    _queue.pop_front();
    _cond_producer.notify_one();
    return true;
}

template<class T>
bool BlockQueue<T>::pop(T& item,int timeout)
{
    std::unique_lock<std::mutex> locker(_mtx);
    while(_queue.empty()){
        if(_cond_consumer.wait_for(locker,std::chrono::seconds(timeout))
            ==std::cv_status::timeout)
        {
            return false;
        }
        if(_isClose)
        {
            return false;
        }
    }
    item=_queue.front();
    _queue.pop_front();
    _cond_producer.notify_one();
    return true;
}

#endif