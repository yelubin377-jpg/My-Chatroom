#include "TimerQueue.h"
#include "EventLoop.h"
#include <sys/timerfd.h>
#include <unistd.h>
#include <string.h>
#include <functional>
#include <muduo/base/Logging.h>

using namespace muduo::net;
using muduo::Timestamp;

namespace
{

int createTimerfd()
{
    int fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd < 0)
    {
        LOG_ERROR << "timerfd_create failed";
    }
    return fd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch()
                         - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / 1000000);
    ts.tv_nsec = static_cast<long>((microseconds % 1000000) * 1000);
    return ts;
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, 0, sizeof(newValue));
    memset(&oldValue, 0, sizeof(oldValue));
    newValue.it_value = howMuchTimeFromNow(expiration);
    ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
}

void readTimerfd(int timerfd)
{
    uint64_t howmany;
    ::read(timerfd, &howmany, sizeof(howmany));
}

}  // namespace

TimerQueue::TimerQueue(EventLoop* loop)
    : _loop(loop),
      _timerfd(createTimerfd()),
      _channel(loop, _timerfd),
      _callingExpiredTimers(false)
{
    _channel.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    _channel.enableReading();
}

TimerQueue::~TimerQueue()
{
    _channel.disableAll();
    _channel.remove();
    ::close(_timerfd);
}

void TimerQueue::addTimer(std::function<void()> cb, Timestamp when, double interval)
{
    Timer* timer = new Timer(std::move(cb), when, interval);
    bool earliestChanged = _timers.empty() || when < _timers.begin()->first;
    _timers.insert(Entry(when, timer));

    if (earliestChanged)
    {
        resetTimerfd(_timerfd, when);
    }
}

void TimerQueue::handleRead()
{
    Timestamp now(Timestamp::now());
    readTimerfd(_timerfd);

    std::vector<Entry> expired = getExpired(now);

    _callingExpiredTimers = true;
    for (const Entry& entry : expired)
    {
        entry.second->run();
    }
    _callingExpiredTimers = false;

    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    // 哨兵：UINTPTR_MAX保证大于任何合法Timer指针
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerSet::iterator end = _timers.lower_bound(sentry);

    std::copy(_timers.begin(), end, back_inserter(expired));
    _timers.erase(_timers.begin(), end);
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    for (const Entry& entry : expired)
    {
        if (entry.second->repeat())
        {
            Timestamp nextExpire = now.addSeconds(entry.second->interval());
            entry.second->restart(nextExpire);
            _timers.insert(Entry(nextExpire, entry.second));
        }
        else
        {
            delete entry.second;
        }
    }

    if (!_timers.empty())
    {
        Timestamp nextExpire = _timers.begin()->first;
        if (nextExpire.valid())
        {
            resetTimerfd(_timerfd, nextExpire);
        }
    }
}
void TimerQueue::cancel(TimerId timerId)
{
    Timer* timer = timerId.timer();
    if(timer)
    {
        Entry target(timer->expiration(), timer);
        auto it = _timers.find(target);
        if(it != _timers.end() && it->second == timer)
        {
            _timers.erase(it);
            delete timer;
        }
    }
}
