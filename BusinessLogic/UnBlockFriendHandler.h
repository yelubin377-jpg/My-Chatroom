#pragma once
#include <muduo/net/TcpConnection.h>
class MyProtoMsg;
void UnBlockFriendHandler(const muduo::net::TcpConnectionPtr& conn,
                          const MyProtoMsg& msg,
                          void* ctx);