#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "SocketsOps.h"
#include <functional>
#include "EventLoopThreadPool.h"
#include <muduo/base/Logging.h>

using namespace muduo::net;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr,
                     const std::string& name)
    : _loop(loop),
      _name(name),
      _acceptor(new Acceptor(loop, listenAddr)),
      _nextConnId(1)
      //
      ,_threadPool(new EventLoopThreadPool(loop))
      //
{
    _acceptor->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this,
                  std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
}

void TcpServer::start()
{
    //
    _threadPool->start();
    //
    _acceptor->listen();
    LOG_INFO << "TcpServer [" << _name << "] started";
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%s#%d", _name.c_str(), _nextConnId++);
    std::string connName = buf;

    struct sockaddr_in local = sockets::getsockname(sockfd);
    InetAddress localAddr(local);
    EventLoop* ioLoop = _threadPool->getNextLoop();
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));

    conn->setConnectionCallback(_connectionCallback);
    conn->setMessageCallback(_messageCallback);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    _connections[connName] = conn;
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    _connections.erase(conn->name());
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}
void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    _loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}


