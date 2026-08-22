#include "discard.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpServer.h"
#include "muduo/base/Logging.h"
#include <unistd.h>
int main()
{
    LOG_INFO << "pid = " <<getpid();
    muduo::net::EventLoop loop;
    muduo::net::InetAddress listenAddr(2025);
    DiscardServer server(&loop,listenAddr);
    server.start();
    loop.loop();
}