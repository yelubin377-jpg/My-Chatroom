#pragma once
#include <muduo/net/TcpConnection.h>
class MyProtoMsg;
void QueryGroupMembersHandler(const muduo::net::TcpConnectionPtr& conn,
                              const MyProtoMsg& msg,
                              void* ctx);