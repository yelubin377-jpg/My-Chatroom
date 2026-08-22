#include "daytime.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Timestamp.h"

DaytimeServer::DaytimeServer(muduo::net::EventLoop* loop,
                             const muduo::net::InetAddress& listenAddr)
                : _loop(loop)
                ,_server(loop,listenAddr,"DaytimeServer")
{
    _server.setConnectionCallback(
        [this](const muduo::net::TcpConnectionPtr& conn)
        {
            onConnection(conn);
        });
}

void DaytimeServer::start()
{
    _server.start();
}


void DaytimeServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if(conn -> connected())
    {
        muduo::Timestamp now = muduo::Timestamp::now(); 
        conn -> send(now.toFormattedString());
        conn -> shutdown();
    }
    LOG_INFO << "DaytimeServer - " << conn->peerAddress().toIpPort()
             << " -> " << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    
}