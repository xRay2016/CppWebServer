/*
 * @Author: your name
 * @Date: 2021-09-08 11:52:47
 * @LastEditTime: 2021-09-08 17:09:33
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/src/heaptimer.cpp
 */
#include "heaptimer.h"
#include <assert.h>

void HeapTimer::siftup_(size_t i)
{
    assert(i>=0 && i<_heap.size());
    size_t parent=(i-1)/2;
    while(parent>=0){
        if(_heap[parent]<_heap[i]){break;}
        SwapNode(i,parent);
        i=parent;
        parent=(i-1)/2;
    }
}

bool HeapTimer::siftdown_(size_t index,size_t n)
{
    assert(index>=0 && index<_heap.size());
    assert(n>=0 && n<=_heap.size());
    size_t i=index;
    size_t j=2*i+1;
    while(j<n){
        if(j+1<n && _heap[j+1]<_heap[j]){
            j++;
        }
        if(_heap[i]<_heap[j]){
            break;
        }
        SwapNode(i,j);
        i=j;
        j=i*2+1;
    }   
    return i>index;
}

void HeapTimer::SwapNode(size_t i,size_t j)
{
    assert(i>=0 && i<_heap.size());
    assert(j>=0 && j<_heap.size());
    std::swap(_heap[i],_heap[j]);
    _ref[_heap[i].id]=i;
    _ref[_heap[j].id]=j;
}

void HeapTimer::add(int id,int timeout,const TimeoutCallBack& cb)
{
    assert(id>=0);
    size_t i;
    if(_ref.count(id)==0){
        i=_heap.size();
        _ref[id]=i;
        _heap.push_back({id,Clock::now()+MS(timeout),cb});
        siftup_(i);
    }
    else{
        i=_ref[id];
        _heap[i].expires=Clock::now()+MS(timeout);
        _heap[i].cb=cb;
        if(!siftdown_(i,_heap.size())){
            siftup_(i);
        }
    }
}

void HeapTimer::doWork(int id)
{
    /*  删除指定id的节点，并触发回调函数  */
    if(_heap.empty()||_ref.count(id)==0){
        return;
    }
    size_t i=_ref[id];
    TimerNode node=_heap[i];
    node.cb();
    del_(i);
}

void HeapTimer::del_(size_t index)
{
    assert(!_heap.empty() && index>=0 && index<_heap.size());
    size_t n=_heap.size()-1;
    SwapNode(index,n);
    if(!siftdown_(index,n))
    {
        siftup_(index);
    }
    _ref.erase(_heap.back().id);
    _heap.pop_back();
}

void HeapTimer::adjust(int id,int timeout)
{
    assert(!_heap.empty()&&_ref.count(id)>0);
    size_t i=_ref[id];
    _heap[i].expires=Clock::now()+MS(timeout);
    siftdown_(_ref[id],_heap.size());
}

void HeapTimer::tick()
{
    if(_heap.empty()){
        return;
    }
    while(!_heap.empty()){
        TimerNode node=_heap.front();
        if(std::chrono::duration_cast<MS>(node.expires-Clock::now()).count()>0){
            break;
        }
        node.cb();
        pop();
    }
}

void HeapTimer::pop()
{
    assert(!_heap.empty());
    del_(0);
}

void HeapTimer::clear(){
    _ref.clear();
    _heap.clear();
}

int HeapTimer::GetNextTick()
{
    tick();
    size_t res=-1;
    if(!_heap.empty()){
        res=std::chrono::duration_cast<MS>(_heap.front().expires-Clock::now()).count();
        if(res<0) res=0;
    }
    return res;
}
