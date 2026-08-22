#include "chargen.h"
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include <unistd.h>

// chargen 测试: ./chargen → nc localhost 2019 | head
int main()
{
  LOG_INFO << "pid = " << getpid();

  muduo::net::EventLoop loop;
  muduo::net::InetAddress listenAddr(2019);   // chargen 标准端口是 19，测试用 2019
  ChargenServer server(&loop,listenAddr);

  server.start();   // 内部: socket → bind(2019) → listen → 注册到 EventLoop
  loop.loop();      // 进入事件循环，等待客户端连接后开始灌字符
}
