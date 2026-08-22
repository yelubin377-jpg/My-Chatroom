#pragma once
#include <stdint.h>
#include <map>
#include <functional>
#include <muduo/net/TcpConnection.h>
class MyProtoMsg;
using MsgHandler = std::function<void(
    const muduo::net::TcpConnectionPtr&,  ///谁传的消息
    const MyProtoMsg&,                    ///解析好的消息
    void*                                 ///服务器指针（可以拿链接列表，redis）
)>;

class Router // 路由   分配者 - 给消息类型号分对应的处理函数
{
public:
    void on(uint16_t type,MsgHandler handler);
    void dispatch(uint16_t type,
                  const muduo::net::TcpConnectionPtr& conn,   //指针
                  const MyProtoMsg& msg,
                  void* ctx);
private:
    std::map<uint16_t,MsgHandler> _handlers;    
};