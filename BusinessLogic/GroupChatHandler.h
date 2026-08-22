#pragma once
#include <muduo/net/TcpConnection.h>
class MyProtoMsg;
void GroupChatHandler(const muduo::net::TcpConnectionPtr& conn,
                      const MyProtoMsg& msg,
                      void* ctx);