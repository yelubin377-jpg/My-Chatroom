#include "TcpClient.h"
#include "Connector.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "SocketsOps.h"
#include <functional>
#include <muduo/base/Logging.h>

using namespace muduo::net;

TcpClient::TcpClient(EventLoop* loop, const InetAddress& serverAddr,
                     const std::string& name)
    : _loop(loop),
      _name(name),
      _serverAddr(serverAddr),
      _connector(new Connector(loop, serverAddr)),
      _connect(false)
{
    _connector->setNewConnectionCallback(
        std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
}

TcpClient::~TcpClient()
{
}

void TcpClient::connect()
{
    _connect = true;
    _connector->start();
}

void TcpClient::disconnect()
{
    _connect = false;
    if (_connection)
    {
        _connection->shutdown();
    }
}

void TcpClient::newConnection(int sockfd)
{
    InetAddress localAddr(sockets::getsockname(sockfd));
    InetAddress peerAddr(_serverAddr);

    char buf[64];
    snprintf(buf, sizeof(buf), "%s#%d", _name.c_str(), sockfd);
    TcpConnectionPtr conn(
        new TcpConnection(_loop, buf, sockfd, localAddr, peerAddr));

    conn->setConnectionCallback(_connectionCallback);
    conn->setMessageCallback(_messageCallback);
    conn->setCloseCallback(
        std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));

    _connection = conn;
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
    _connection.reset();
    _loop->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}
