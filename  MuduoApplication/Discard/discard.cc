#include "discard.h"
#include "muduo/base/Logging.h"
#include <boost/bind.hpp>

DiscardServer::DiscardServer(muduo::net::EventLoop* loop,
                             const muduo::net::InetAddress& listenAddr)
                             : _loop(loop)
                             , _server(loop,listenAddr,"DiscardServer")
{
        _server.setConnectionCallback(boost::bind(&DiscardServer::onConnection,this,_1));
        _server.setMessageCallback(boost::bind(&DiscardServer::onMessage,this,_1,_2,_3));
}

void DiscardServer::start()
{
    _server.start();
}

void DiscardServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    LOG_INFO << "DiscardServer - " << conn->peerAddress().toIpPort()
             << " -> " << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
}

void DiscardServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
               muduo::net::Buffer* buf,
               muduo::Timestamp receiveTime)
{
    buf->retrieveAll();          
}