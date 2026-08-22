#pragma once
#include <functional>
#include <memory>
#include <muduo/base/noncopyable.h>
#include "InetAddress.h"

namespace muduo
{
namespace net
{

class EventLoop;
class Channel;

class Connector : noncopyable,
                  public std::enable_shared_from_this<Connector>
{
public:
    typedef std::function<void(int sockfd)> NewConnectionCallback;

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void start();
    void restart();
    void stop();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { _newConnectionCallback = cb; }

private:
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    void resetChannel();

    enum States { kDisconnected, kConnecting, kConnected };

    EventLoop* _loop;
    InetAddress _serverAddr;
    States _state;
    std::unique_ptr<Channel> _channel;
    NewConnectionCallback _newConnectionCallback;
    int _retryDelayMs;
};

}
}
