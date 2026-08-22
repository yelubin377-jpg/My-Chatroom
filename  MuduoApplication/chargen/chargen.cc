#include "chargen.h"
#include "muduo/base/Logging.h"

// ── 构造函数 ───────────────────────────────────────────────
ChargenServer::ChargenServer(muduo::net::EventLoop* loop,
                             const muduo::net::InetAddress& listenAddr)
    : _loop(loop)
    ,_server(loop,listenAddr,"ChargenServer")
    ,_transferred(0)                  // 统计归零
{
  // —— 造那 95 个可打印字符 ——
  // ASCII 可打印字符: 空格=32, '~'=126, 共 95 个
  _message.reserve(95);               // 预分配空间，避免多次扩容
  for(int c = 32; c <= 126; c++)
  {
    _message.push_back((char)c);      // 把 ASCII 码转成字符逐个塞进去
  }
  // _message 现在是: " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDE...xyz{|}~"

  // —— 注册三个回调：连接、消息、写完成 ——
  _server.setConnectionCallback(
      [this](const muduo::net::TcpConnectionPtr& conn) {
        onConnection(conn);
      });
  _server.setMessageCallback(
      [this](const muduo::net::TcpConnectionPtr& conn,
             muduo::net::Buffer* buf,
             muduo::Timestamp time) {
        onMessage(conn,buf,time);
      });
  _server.setWriteCompleteCallback(   // ← 新增：输出缓冲区清空时触发
      [this](const muduo::net::TcpConnectionPtr& conn) {
        onWriteComplete(conn);
      });
}

// ── 启动 ───────────────────────────────────────────────────
void ChargenServer::start()
{
  _server.start();
}

// ── 连接回调 ───────────────────────────────────────────────
void ChargenServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
  LOG_INFO << "ChargenServer - " << conn->peerAddress().toIpPort()
           << " -> " << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");

  if(conn->connected())
  {
    // setTcpNoDelay: 禁用 Nagle 算法，强制立即发送
    // 默认 TCP 会攒够一批数据再发以节省带宽
    // chargen 要立刻灌数据出去，不能等攒够
    conn->setTcpNoDelay(true);

    conn->send(_message);             // 发第一批 95 个字符
    // 发完后 muduo 触发 onWriteComplete → 继续发下一批 → 发完再触发...
  }
}

// ── 消息回调：chargen 不关心客户端发了什么 ─────────────────
void ChargenServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
                              muduo::net::Buffer* buf,
                              muduo::Timestamp time)
{
  muduo::string msg(buf->retrieveAllAsString()); // 读出来看看
  LOG_INFO << conn->name() << " discards " << msg.size()
           << " bytes received at " << time.toString();  // 记日志
  // 不做任何回复——chargen 只灌不接
}

// ── ★ 写完成回调 ★ ─────────────────────────────────────────
// 调用时机：conn->send() 不是同步发出去的，数据先放进 muduo 的输出缓冲区
//   muduo 异步往内核发，发完了缓冲区空了 → 调这个函数
// "上一批 95 个字符发完了，还要发吗？" → "发！继续发！"
void ChargenServer::onWriteComplete(const muduo::net::TcpConnectionPtr& conn)
{
  _transferred += _message.size();    // 统计累计发送量
  conn->send(_message);               // 继续灌下一批 95 个字符
  // 发完 → 再触发 onWriteComplete → 再发 → ... 死循环
  // 直到客户端断开连接或网络出错
}
