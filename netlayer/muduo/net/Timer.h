// -*- mode: c++ -*-
#pragma once
#include <muduo/base/noncopyable.h>
#include <muduo/base/Timestamp.h>
#include <functional>

namespace muduo
{
namespace net
{

class Timer : noncopyable
{
public:
    using TimerCallback = std::function<void()>;

    Timer(TimerCallback cb , Timestamp when , double interval)
        : _callback(std::move(cb))
        , _expiration(when)
        , _interval(interval)
        , _repeat(interval > 0.0)
    {}

    void run() const {_callback();}
    Timestamp expiration() const {return _expiration;}
    bool repeat() const {return _repeat;}
    double interval() const {return _interval;}
    void restart(Timestamp now) { _expiration = now; }

private:
    TimerCallback _callback;
    Timestamp _expiration;
    double _interval;
    bool _repeat;
};

}
}
