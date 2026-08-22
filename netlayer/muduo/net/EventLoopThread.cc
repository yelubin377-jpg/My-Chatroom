#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace muduo::net;

EventLoopThread::EventLoopThread()
    : _loop(nullptr)
{
}

EventLoopThread::~EventLoopThread()
{
    if(_loop != nullptr)
    {
        _loop->quit();
        _thread.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    _thread = std::thread(&EventLoopThread::threadFunc, this);
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while(_loop == nullptr)
        {
            _cond.wait(lock);
        }
        loop = _loop;
    }
    return loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _loop = &loop;
        _cond.notify_one();
    }
    loop.loop();
    _loop = nullptr;
}

