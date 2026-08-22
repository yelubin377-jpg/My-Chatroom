#include "Poller.h"
#include "Channel.h"
#include <fcntl.h>
#include <unistd.h>

using namespace muduo::net;

Poller::Poller(EventLoop* loop)
    : _ownerLoop(loop),
      _epollfd(::epoll_create1(EPOLL_CLOEXEC)),   //创建epoll实例，返回一个数字fd来代表这个实例
      _events(16)
{}

Poller::~Poller()
{
    ::close(_epollfd);
}

void Poller::updateChannel(Channel* channel)
{
    int fd = channel->fd();
    int op = _channels.count(fd);
    if(!op)
    {
        op = EPOLL_CTL_ADD;
    }else op=EPOLL_CTL_MOD;
    struct epoll_event ev;
    ev.events = channel->events();
    ev.data.ptr = channel;
    ::epoll_ctl(_epollfd, op, fd, &ev);
    _channels[fd] = channel;
}

void Poller::removeChannel(Channel* channel)
{
    int fd = channel->fd();
    _channels.erase(fd);
    ::epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, nullptr);
}

std::vector<Channel*> Poller::poll(int timeoutMs)
{
    std::vector<Channel*> activeChannels;
    int numEvents = ::epoll_wait(_epollfd, _events.data(),
                                 _events.size(), timeoutMs);
    if (numEvents > 0)
    {
        for (int i = 0; i < numEvents; i++)
        {
            Channel* channel = static_cast<Channel*>(_events[i].data.ptr);
            channel->setReallyevents(_events[i].events);
            activeChannels.push_back(channel);
        }
    }
    return activeChannels;
}

bool Poller::hasChannel(Channel* channel) const
{
    return _channels.find(channel->fd()) != _channels.end();
}
