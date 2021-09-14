/*
 * @Author: your name
 * @Date: 2021-09-08 11:40:22
 * @LastEditTime: 2021-09-08 16:30:13
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/include/heaptimer.h
 */
#ifndef HEAPTIMER_H
#define HEAPTIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <functional>
#include <chrono>
#include <vector>
#include <unordered_map>

using TimeoutCallBack=std::function<void()>;
using Clock=std::chrono::high_resolution_clock;
using MS=std::chrono::milliseconds;
using TimeStamp=Clock::time_point;

struct TimerNode{
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;
    bool operator<(const TimerNode& t){
        return expires<t.expires;
    }
};

class HeapTimer{
public:
    HeapTimer() { _heap.reserve(64); }

    ~HeapTimer() { clear(); }
    
    void adjust(int id, int newExpires);

    void add(int id, int timeOut, const TimeoutCallBack& cb);

    void doWork(int id);

    void clear();

    void tick();

    void pop();

    int GetNextTick();

private:
    void del_(size_t i);

    void siftup_(size_t i);

    bool siftdown_(size_t index,size_t n);
    
    void SwapNode(size_t i,size_t j);

    std::vector<TimerNode> _heap;

    std::unordered_map<int,size_t> _ref;
};

#endif