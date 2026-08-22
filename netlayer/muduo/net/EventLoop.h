#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <functional>
#include <muduo/base/noncopyable.h>
#include <muduo/base/Timestamp.h>
#include "TimerId.h"
#include "Callbacks.h"

namespace muduo
{
namespace net
{

class Channel;
class Poller;
class TimerQueue;

class EventLoop : noncopyable
{
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    // runEvery: 重复定时器，聊天室心跳检测用的就是这个
    // 见《Linux多线程服务端编程》8.2节，timerfd+set的实现
    TimerId runEvery(double interval, TimerCallback cb);
    void runInLoop(std::function<void()> cb);
    void queueInLoop(std::function<void()> cb);
    void cancel(TimerId timerId);
    void wakeup();
    void assertInLoopThread() const;

private:
    void handleRead();
    void doPendingFunctors();

    bool _looping;
    bool _quit;
    bool _callingPendingFunctors;

    std::unique_ptr<Poller> _poller;
    std::unique_ptr<TimerQueue> _timerQueue;

    int _wakeupFd;
    std::unique_ptr<Channel> _wakeupChannel;

    std::mutex _mutex;
    std::vector<std::function<void()>> _pendingFunctors;
};

}
}
