#pragma once
#include <muduo/net/TcpConnection.h>
class MyProtoMsg;
void LoginByCodeHandler(const muduo::net::TcpConnectionPtr& conn,
                        const MyProtoMsg& msg,
                        void* ctx);
