#pragma once
#include <muduo/net/TcpConnection.h>
class MyProtoMsg;
void PrivateChatHandler(const muduo::net::TcpConnectionPtr& conn,
                        const MyProtoMsg& msg,
                        void* ctx);
