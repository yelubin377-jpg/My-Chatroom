#pragma once
#include <functional>
#include <muduo/base/noncopyable.h>
#include "Channel.h"
#include "Socket.h"

namespace muduo
{
namespace net
{

class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
public:
    typedef std::function<void(int sockfd, const InetAddress& peerAddr)>
        NewConnectionCallback;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr);

    void listen();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { _newConnectionCallback = cb; }

private:
    void handleRead();

    EventLoop* _loop;
    Socket _acceptSocket;
    Channel _acceptChannel;
    NewConnectionCallback _newConnectionCallback;
};

}
}
