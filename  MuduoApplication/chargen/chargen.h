// RFC 864: 客户端连上来，服务器不停发送 95 个可打印字符，循环往复永不停止
// 这个示例教你三样东西：
//   1. 服务器主动推送（不停发数据）
//   2. onWriteComplete 回调（发完继续发，死循环灌字符）
//   3. TCP_NODELAY（禁用 Nagle，要求立即发送不攒包）
#ifndef MUDUO_EXAMPLES_CHARGEN_CHARGEN_H
#define MUDUO_EXAMPLES_CHARGEN_CHARGEN_H

#include "muduo/net/TcpServer.h"
#include <string>   // std::string 存那 95 个可打印字符

class ChargenServer
{
 public:
  ChargenServer(muduo::net::EventLoop* loop,
                const muduo::net::InetAddress& listenAddr);

  void start();

 private:
  // 有人连上/断开时调。连上后启动字符灌注
  void onConnection(const muduo::net::TcpConnectionPtr& conn);

  // 收到数据时调——chargen 不关心客户端发了啥，打日志然后丢掉
  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp time);

  // ★ 新回调 ★ 发送缓冲区被清空时被调——"你上次 send 的已经发完了，还要发吗？"
  // chargen 在这里可以继续塞数据，形成无限发送循环
  void onWriteComplete(const muduo::net::TcpConnectionPtr& conn);

  muduo::net::EventLoop* _loop;
  muduo::net::TcpServer _server;

  std::string _message;        // 95 个可打印字符拼成的字符串，从 ' ' 到 '~'
  int64_t _transferred;        // 累计已发字节数，统计用
};

#endif
