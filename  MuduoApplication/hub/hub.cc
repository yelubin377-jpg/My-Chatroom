#include "hub.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "muduo/base/Logging.h"
#include <boost/bind/bind.hpp>
using namespace boost::placeholders;
HubServer::HubServer(muduo::net::EventLoop* loop,
              const muduo::net::InetAddress& listenAddress)
            :_loop(loop),
            _server(loop,listenAddress,"HubServer")
{
    _server.setMessageCallback(boost::bind(&HubServer::onMessage,this,_1,_2,_3));
    _server.setConnectionCallback(boost::bind(&HubServer::onConnection,this,_1));
}
void HubServer::start()
{
    _server.start();
}
void HubServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    LOG_INFO << "HubServer(onConnection) - " << conn->peerAddress().toIpPort()
             << " -> " << conn->localAddress().toIpPort()
             << " is " << (conn->connected() ? "UP" : "DOWN");
    if(conn->connected())
    {
        _store.insert(conn);
    }
    else
    {
        _store.erase(conn);
    }
}
void HubServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
               muduo::net::Buffer* buf,
               muduo::Timestamp time)
{
    muduo::string msg(buf->retrieveAllAsString());
    LOG_INFO <<  "HubServer(onMessage) - " << conn->peerAddress().toIpPort()
             << " -> " << conn->localAddress().toIpPort() << msg.size()
             << " bytes received at " << time.toFormattedString();
    for(auto& c:_store) 
    {
        c->send(msg);
    }

}