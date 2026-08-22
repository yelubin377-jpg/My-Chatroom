#pragma once
#include <set>
#include <vector>
#include <memory>
#include <muduo/base/noncopyable.h>
#include <muduo/base/Timestamp.h>
#include "Channel.h"
#include "Timer.h"
#include "TimerId.h"

namespace muduo
{
namespace net
{

class EventLoop;

class TimerQueue : noncopyable
{
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    void addTimer(std::function<void()> cb, Timestamp when, double interval);
    void cancel(TimerId timerId);  // TODO

private:
    typedef std::pair<Timestamp, Timer*> Entry;
    typedef std::set<Entry> TimerSet;

    void handleRead();
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);

    EventLoop* _loop;
    const int _timerfd;
    Channel _channel;
    TimerSet _timers;
    bool _callingExpiredTimers;
};

}
}
