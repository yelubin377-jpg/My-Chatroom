#include "EventLoop.h"
#include "Poller.h"
#include "Channel.h"
#include "TimerQueue.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <signal.h>
#include <muduo/base/Logging.h>

using namespace muduo::net;

namespace
{

int createEventfd()
{
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0)
    {
        LOG_ERROR << "eventfd create failed";
    }
    return fd;
}

}  // namespace

EventLoop::EventLoop()
    : _looping(false),
      _quit(false),
      _callingPendingFunctors(false),
      _poller(new Poller(this)),
      _timerQueue(new TimerQueue(this)),
      _wakeupFd(createEventfd()),
      _wakeupChannel(new Channel(this, _wakeupFd))
{
    // eventfd被写后epoll会感知到可读，用来跨线程唤醒epoll_wait
    _wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this));
    _wakeupChannel->enableReading();
}

EventLoop::~EventLoop()
{
    _wakeupChannel->disableAll();
    _wakeupChannel->remove();
    ::close(_wakeupFd);
}

void EventLoop::loop()
{
    _looping = true;
    _quit = false;

    while (!_quit)
    {
        std::vector<Channel*> activeChannels = _poller->poll(10000);
        for (Channel* channel : activeChannels)
        {
            channel->handleEvent();
        }
        doPendingFunctors();
    }
    _looping = false;
}

void EventLoop::quit()
{
    _quit = true;
    wakeup();
}

void EventLoop::updateChannel(Channel* channel)
{
    _poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    _poller->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    return _poller->hasChannel(channel);
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
    Timestamp when = Timestamp::now().addSeconds(interval);
    _timerQueue->addTimer(std::move(cb), when, interval);
    // TODO: cancel功能——TimerId现在是空的，加了定时器就取消不了
    return TimerId();
}

void EventLoop::runInLoop(std::function<void()> cb)
{
    // 简化版：不区分当前线程，统一走queue
    queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(std::function<void()> cb)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _pendingFunctors.push_back(std::move(cb));
    }
    wakeup();
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ::write(_wakeupFd, &one, sizeof(one));
}

void EventLoop::handleRead()
{
    uint64_t one;
    ::read(_wakeupFd, &one, sizeof(one));
}

void EventLoop::doPendingFunctors()
{
    std::vector<std::function<void()>> functors;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        functors.swap(_pendingFunctors);
    }

    _callingPendingFunctors = true;
    for (auto& func : functors)
    {
        func();
    }
    _callingPendingFunctors = false;
}

void EventLoop::assertInLoopThread() const
{
    // 单线程跑，暂时不检查。多线程的话需要pthread_self()比对
}
void EventLoop::cancel(TimerId timerId)
{
    _timerQueue->cancel(timerId);
}