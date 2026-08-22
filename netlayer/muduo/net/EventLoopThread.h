#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <muduo/base/noncopyable.h>

namespace muduo
{
namespace net
{

class EventLoop;

class EventLoopThread : noncopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();
    EventLoop* _loop;
    std::thread _thread;
    std::mutex _mutex;
    std::condition_variable _cond;
};}}
