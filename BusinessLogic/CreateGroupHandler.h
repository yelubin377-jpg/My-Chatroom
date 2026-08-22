#pragma once
#include <muduo/net/TcpConnection.h>
class MyProtoMsg;
void CreateGroupHandler(const muduo::net::TcpConnectionPtr& conn,
                        const MyProtoMsg& msg,
                        void* ctx);