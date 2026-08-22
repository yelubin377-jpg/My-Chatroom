#include "hub.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "muduo/base/Logging.h"
#include <unistd.h>

int main()
{
    LOG_INFO << " pid = " << getpid();
    muduo::net::EventLoop loop;
    muduo::net::InetAddress listenAddress(2026);
    HubServer server(&loop,listenAddress);
    server.start();
    loop.loop();        
}