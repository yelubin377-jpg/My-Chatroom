#include "ChatServer.h"
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fcntl.h>      // open、O_CREAT
#include <sys/file.h>   // flock、LOCK_EX、LOCK_NB
int main(int argc,char* argv[])
{
    int lock_fd = open("/tmp/chat_server.lock", O_CREAT, 0644);
    if(flock(lock_fd, LOCK_EX | LOCK_NB) != 0)
    {
        printf("服务端已在运行, 本实例退出!\n");
        return 0;
    }
    std::string ip = "0.0.0.0";      
    int port = 2026;                 
    if(argc >= 2) ip = argv[1];      
    if(argc >= 3) port = std::atoi(argv[2]);   
    LOG_INFO << "pid = "<<getpid();
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr(ip,port);
    ChatServer server(&loop, addr);
    server.start();
    loop.loop();
}