#include "Connector.h"
#include "EventLoop.h"
#include "Channel.h"
#include "SocketsOps.h"
#include <unistd.h>
#include <muduo/base/Logging.h>

using namespace muduo::net;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : _loop(loop),
      _serverAddr(serverAddr),
      _state(kDisconnected),
      _retryDelayMs(500)
{
}

Connector::~Connector()
{
}

void Connector::start()
{
    _state = kConnecting;
    connect();
}

void Connector::connect()
{
    int sockfd = sockets::createNonblockingSockfd();
    int ret = ::connect(sockfd, _serverAddr.getSockAddr(),
                        _serverAddr.getSockAddrLen());
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
            break;
        default:
            retry(sockfd);
            break;
    }
}

void Connector::connecting(int sockfd)
{
    _channel.reset(new Channel(_loop, sockfd));
    _channel->setWriteCallback(
        std::bind(&Connector::handleWrite, shared_from_this()));
    _channel->setErrorCallback(
        std::bind(&Connector::handleError, shared_from_this()));
    _channel->enableWriting();
}

void Connector::handleWrite()
{
    if (_state == kConnecting)
    {
        int sockfd = _channel->fd();
        _channel->remove();

        int err = 0;
        socklen_t optlen = sizeof(err);
        ::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &optlen);
        if (err)
        {
            retry(sockfd);
        }
        else
        {
            _state = kConnected;
            if (_newConnectionCallback)
            {
                _newConnectionCallback(sockfd);
            }
        }
    }
}

void Connector::handleError()
{
    int sockfd = _channel->fd();
    _channel->remove();
    retry(sockfd);
}

void Connector::retry(int sockfd)
{
    ::close(sockfd);
    _channel.reset();
    if (_state == kConnecting)
    {
        // FIXME: 重试没有用_retryDelayMs，直接就连了
        _loop->runInLoop(
            std::bind(&Connector::connect, shared_from_this()));
    }
}

void Connector::stop()
{
    _state = kDisconnected;
    if (_channel)
    {
        _channel->remove();
    }
}

void Connector::restart()
{
    stop();
    start();
}

void Connector::resetChannel()
{
    _channel.reset();
}
