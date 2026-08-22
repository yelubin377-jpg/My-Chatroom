#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

using namespace muduo::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
    : _baseLoop(baseLoop),
      _started(false),
      _numThreads(0),
      _next(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start()
{
    _started = true;
    for(int i = 0; i < _numThreads; i++)
    {
        std::unique_ptr<EventLoopThread> t(new EventLoopThread());
        _loops.push_back(t->startLoop());
        _threads.push_back(std::move(t));
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    EventLoop* loop = _baseLoop;
    if(!_loops.empty())
    {
        loop = _loops[_next];
        _next = (_next + 1) % static_cast<int>(_loops.size());
    }
    return loop;
}


