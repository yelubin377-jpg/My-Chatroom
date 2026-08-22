#pragma once
#include <map>
#include <string>
#include <memory>
#include <muduo/base/noncopyable.h>
#include "Callbacks.h"
#include "InetAddress.h"
#include "EventLoopThreadPool.h"
//
class EventLoopThreadPool;
#include <memory>
//
namespace muduo
{
namespace net
{

class EventLoop;
class Acceptor;
class TcpConnection;

class TcpServer : noncopyable
{
public:
    TcpServer(EventLoop* loop, const InetAddress& listenAddr,
              const std::string& name);
    ~TcpServer();

    void start();

    void setConnectionCallback(const ConnectionCallback& cb)
    { _connectionCallback = cb; }
    void setMessageCallback(const MessageCallback& cb)
    { _messageCallback = cb; }
    //
    void setThreadNum(int numThreads) { _threadPool->setThreadNum(numThreads); }
    //

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    EventLoop* _loop;
    const std::string _name;
    std::unique_ptr<Acceptor> _acceptor;
    std::unique_ptr<EventLoopThreadPool> _threadPool;
    ConnectionCallback _connectionCallback;
    MessageCallback _messageCallback;

    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
    ConnectionMap _connections;
    int _nextConnId;
};

}
}
