#pragma once
#include <functional>
#include <memory>
#include <sys/epoll.h>
#include <muduo/base/noncopyable.h>

namespace muduo
{
namespace net
{
class EventLoop;
class Channel : noncopyable //fd出问题
{
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd)
        : _loop(loop),
          _fd(fd),
         _reallyevents(0),
          _events(0),
          _index(-1)
    {}

    int fd() const { return _fd; }
    int events() const { return _events; }
    void setReadCallback(EventCallback cb)  { _readCallback = std::move(cb); }
    void setWriteCallback(EventCallback cb) { _writeCallback = std::move(cb); }
    void setCloseCallback(EventCallback cb) { _closeCallback = std::move(cb); }



    int index() const { return _index; }
    void setIndex(int index) { _index = index; }
    void setReallyevents(int reallyevents) { _reallyevents = reallyevents; }
    bool whetherNoneevents() const { return _events == 0; }
    void enableReading()  { _events |= _YReadEvent; update(); }
    void disableReading() { _events &= ~_YReadEvent; update(); }
    void enableWriting()  { _events |= _YWriteEvent; update(); }
    void disableWriting() { _events &= ~_YWriteEvent; update(); }
    void disableAll()     { _events = _YNoneEvent; update(); }
    bool isReading() const { return _events & _YReadEvent; }
    bool isWriting() const { return _events & _YWriteEvent; }


    void setErrorCallback(EventCallback cb) { _errorCallback = std::move(cb); }

    // tie: 防止channel回调时TcpConnection已经被析构
    void tie(const std::shared_ptr<void>&);

    void handleEvent();
    void remove();

private:
    EventLoop* _loop;
    const int _fd;
    void update();
    int _reallyevents;
    int _events;
    EventCallback _readCallback;
    static const int _YReadEvent = EPOLLIN | EPOLLPRI;
    EventCallback _writeCallback;
    static const int _YWriteEvent = EPOLLOUT;
    EventCallback _closeCallback;
    static const int _YCloseEvent = EPOLLHUP|EPOLLERR;
    static const int _YNoneEvent = 0;
    EventCallback _errorCallback;

    




    bool _eventHandling = false;
    bool _addedToLoop = false;
    int _index;
};

}}
