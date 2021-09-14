/*
 * @Author: your name
 * @Date: 2021-08-30 20:52:44
 * @LastEditTime: 2021-09-09 14:11:36
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/code/pool/threadpool.h
 */
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
#include <assert.h>

class ThreadPool{
public:
    explicit ThreadPool(size_t threadCount=8):pool_(std::make_shared<Pool>()){
        assert(threadCount>0);
        for(size_t i=0;i<threadCount;i++){
            std::thread([pool=pool_]{
                std::unique_lock<std::mutex> locker(pool->mtx);
                while(true){
                    if(!pool->tasks.empty()){
                        auto task=std::move(pool->tasks.front());
                        pool->tasks.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    }
                    else if(pool->isClosed)
                        break;
                    else
                        pool->cond.wait(locker);
                }
            }).detach();
        }
    }

    ThreadPool()=default;
    
    ThreadPool(ThreadPool&&)=default;

    ~ThreadPool()
    {
        if(static_cast<bool>(pool_)){
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed=true;
            }
            pool_->cond.notify_all();
        }
    }

    template<class F>
    void AddTask(F&& Task){
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(Task));
        }
        pool_->cond.notify_one();
    }

private:
    struct Pool
    {
        /* data */
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> pool_;
};

#endif