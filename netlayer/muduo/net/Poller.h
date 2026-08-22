#pragma once
#include <vector>
#include <map>
#include <sys/epoll.h>
#include <muduo/base/noncopyable.h>

namespace muduo
{
namespace net
{

class EventLoop;
class Channel;

class Poller : noncopyable
{
public:
    explicit Poller(EventLoop* loop);
    ~Poller();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    std::vector<Channel*> poll(int timeoutMs);
    bool hasChannel(Channel* channel) const;

private:
    EventLoop* _ownerLoop;
    int _epollfd;
    std::vector<struct epoll_event> _events;
    std::map<int, Channel*> _channels;    //查哪些注册过
};

}}
