#ifndef MUDUO_EXAMPLES_DAYTIME_DAYTIME_H//如果没定义再定义，防止重定义
#define MUDUO_EXAMPLES_DAYTIME_DAYTIME_H
#include "muduo/net/TcpServer.h"  //_server需要这个头文件
class DaytimeServer
{
public:
    DaytimeServer(muduo::net::EventLoop* loop,
                  const muduo::net::InetAddress& listenAddr);
    void start();
private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn);
    muduo::net::EventLoop* _loop;  //事件循环
    muduo::net::TcpServer _server;  //网络层全权丢给他
};
#endif