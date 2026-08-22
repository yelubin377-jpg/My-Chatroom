#pragma once
#include <vector>
#include <memory>
#include <muduo/base/noncopyable.h>

namespace muduo
{
namespace net
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    EventLoopThreadPool(EventLoop* baseLoop);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads) { _numThreads = numThreads; }
    void start();
    EventLoop* getNextLoop();
    bool started() const { return _started; }   // 博客写成了 start(),是手误

private:
    EventLoop* _baseLoop;
    bool _started;
    int _numThreads;
    int _next;
    std::vector<std::unique_ptr<EventLoopThread>> _threads;
    std::vector<EventLoop*> _loops;
};}}


