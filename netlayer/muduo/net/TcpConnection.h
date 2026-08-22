#pragma once
#include <functional>
#include <memory>
#include <muduo/base/noncopyable.h>
#include "Callbacks.h"
#include "InetAddress.h"
#include "Buffer.h"

namespace muduo
{
namespace net
{

class EventLoop;
class Channel;
class Socket;

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
public:
    enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected };

    TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                  const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TcpConnection();

    void connectEstablished();
    void connectDestroyed();

    void send(const void* data, size_t len);
    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb)
    { _connectionCallback = cb; }
    void setMessageCallback(const MessageCallback& cb)
    { _messageCallback = cb; }
    void setCloseCallback(const CloseCallback& cb)
    { _closeCallback = cb; }

    bool connected() const { return _state == kConnected; }
    const InetAddress& peerAddress() const { return _peerAddr; }
    const InetAddress& localAddress() const { return _localAddr; }
    const std::string& name() const { return _name; }
    EventLoop* getLoop() const { return _loop; }

private:
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop(const std::string& msg);
    void shutdownInLoop();

    EventLoop* _loop;
    const std::string _name;
    StateE _state;
    std::unique_ptr<Socket> _socket;
    std::unique_ptr<Channel> _channel;
    const InetAddress _localAddr;
    const InetAddress _peerAddr;
    ConnectionCallback _connectionCallback;
    MessageCallback _messageCallback;
    CloseCallback _closeCallback;
    Buffer _inputBuffer;
    Buffer _outputBuffer;
};

}
}
