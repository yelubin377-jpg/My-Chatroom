#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include <muduo/base/Logging.h>

using namespace muduo::net;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
    : _loop(loop),
      _acceptSocket(sockets::createNonblockingSockfd()),
      _acceptChannel(loop, _acceptSocket.fd())
{
    sockets::setReuseAddr(_acceptSocket.fd(), true);  //地址复用
    _acceptSocket.bind(listenAddr);
    _acceptChannel.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen()
{
    _acceptSocket.listen();
    _acceptChannel.enableReading();
    LOG_INFO << "Acceptor listening on fd " << _acceptSocket.fd();
}

void Acceptor::handleRead()
{
    InetAddress peerAddr;
    Socket::Accept_Result result = _acceptSocket.accept();
    if (result.socket >= 0)
    {
        if (_newConnectionCallback)
        {
            _newConnectionCallback(result.socket, result.peerAddr);
        }
        else
        {
            ::close(result.socket);
        }
    }
}
