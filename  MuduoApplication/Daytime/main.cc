#include "daytime.h"
#include<muduo/net/EventLoop.h>
#include<muduo/base/Logging.h>
#include<muduo/net/InetAddress.h>
#include<unistd.h>
int main()
{
    LOG_INFO << "pid = " << getpid();
    muduo::net::EventLoop loop;
    muduo::net::InetAddress listenAddr(2013);
    DaytimeServer server(&loop,listenAddr);
    server.start();
    loop.loop();
}