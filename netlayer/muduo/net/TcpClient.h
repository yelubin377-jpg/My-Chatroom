#pragma once
#include <memory>
#include <string>
#include <muduo/base/noncopyable.h>
#include "Callbacks.h"
#include "InetAddress.h"

namespace muduo
{
namespace net
{

class EventLoop;
class Connector;
class TcpConnection;

class TcpClient : noncopyable
{
public:
    TcpClient(EventLoop* loop, const InetAddress& serverAddr,
              const std::string& name);
    ~TcpClient();

    void connect();
    void disconnect();

    void setConnectionCallback(const ConnectionCallback& cb)
    { _connectionCallback = cb; }
    void setMessageCallback(const MessageCallback& cb)
    { _messageCallback = cb; }

    TcpConnectionPtr connection() const { return _connection; }
    EventLoop* getLoop() const { return _loop; }

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop* _loop;
    std::string _name;
    InetAddress _serverAddr;
    std::shared_ptr<Connector> _connector;
    ConnectionCallback _connectionCallback;
    MessageCallback _messageCallback;
    TcpConnectionPtr _connection;
    bool _connect;
};

}
}
