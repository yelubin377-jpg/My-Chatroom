#pragma once
#ifndef HUB_H
#define HUB_H

#include "muduo/net/TcpServer.h"



#include <set>

class HubServer
{
public:
    HubServer(muduo::net::EventLoop* loop,
              const muduo::net::InetAddress& listenAddress);

    void start();
private:
    muduo::net::TcpServer _server;
    muduo::net::EventLoop* _loop;
    void onConnection(const muduo::net::TcpConnectionPtr& conn);
    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp time);
    std::set<muduo::net::TcpConnectionPtr> _store;
};
#endif