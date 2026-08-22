#include "TcpConnection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include <unistd.h>
#include <functional>
#include <muduo/base/Logging.h>

using namespace muduo::net;

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                             const InetAddress& localAddr, const InetAddress& peerAddr)
    : _loop(loop),
      _name(name),
      _state(kConnecting),
      _socket(new Socket(sockfd)),
      _channel(new Channel(loop, sockfd)),
      _localAddr(localAddr),
      _peerAddr(peerAddr)
{
    _channel->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    _channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    _channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    _channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::connectEstablished()
{
    _state = kConnected;
    _channel->enableReading();

    if (_connectionCallback)
    {
        _connectionCallback(shared_from_this());
    }
}

void TcpConnection::connectDestroyed()
{
    if (_state == kConnected)
    {
        _state = kDisconnected;
        _channel->disableAll();
        if (_connectionCallback)
        {
            _connectionCallback(shared_from_this());
        }
    }
    _channel->remove();
}

void TcpConnection::handleRead()
{
    int savedErrno = 0;
    ssize_t n = _inputBuffer.readFd(_channel->fd(), &savedErrno);
    if (n > 0)
    {
        if (_messageCallback)
        {
            _messageCallback(shared_from_this(), &_inputBuffer, Timestamp::now());
        }
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if (_channel->isWriting())
    {
        ssize_t n = ::write(_channel->fd(),
                            _outputBuffer.peek(),
                            _outputBuffer.readableBytes());
        if (n > 0)
        {
            _outputBuffer.retrieve(n);
            if (_outputBuffer.readableBytes() == 0)
            {
                _channel->disableWriting();
            }
        }
    }
}

void TcpConnection::handleClose()
{
    _state = kDisconnected;
    _channel->disableAll();

    if (_closeCallback)
    {
        _closeCallback(shared_from_this());
    }
}

void TcpConnection::handleError()
{
    handleClose();
}

void TcpConnection::send(const void* data, size_t len)
{
    if (_state != kConnected)
    {
        return;
    }
    std::string msg(static_cast<const char*>(data), len);   // ★ 拷贝!内存被调用方删了也不怕
    _loop->runInLoop(std::bind(&TcpConnection::sendInLoop, shared_from_this(), msg));
}

void TcpConnection::sendInLoop(const std::string& msg)
{
    if (_state != kConnected) return;

    if (!_channel->isWriting() && _outputBuffer.readableBytes() == 0)
    {
        ssize_t n = ::write(_channel->fd(), msg.data(), msg.size());
        if (n >= 0)
        {
            if (static_cast<size_t>(n) >= msg.size())
            {
                return;
            }
            _outputBuffer.append(msg.data() + n, msg.size() - n);
        }
        else
        {
            _outputBuffer.append(msg.data(), msg.size());
        }
        _channel->enableWriting();
        return;
    }

    _outputBuffer.append(msg.data(), msg.size());
    _channel->enableWriting();
}


void TcpConnection::shutdown()
{
    if (_state == kConnected)
    {
        _state = kDisconnecting;
        _loop->runInLoop(
            std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
    }
}

void TcpConnection::shutdownInLoop()
{
    if (!_channel->isWriting())
    {
        _socket.reset();
    }
}
