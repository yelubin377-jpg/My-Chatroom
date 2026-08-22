#include "echo.h"
#include "muduo/base/Logging.h"   //日志
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include <unistd.h>   //getpid 进程号
int main()
{
    LOG_INFO << "pid = " << getpid();
    muduo::net::EventLoop loop; //一有消息就通知你，不停循环看有没有消息
    muduo::net::InetAddress listenAddr(2007);
    EchoServer server(&loop,listenAddr);
    //TcpServer准备好了
    
    server.start();//1.创建socket绑定2007端口，开始监听 2.把socket对应的事件注册到Eventloop
    loop.loop();
}
