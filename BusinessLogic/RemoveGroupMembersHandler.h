#pragma once
#include <muduo/net/TcpConnection.h>
class MyProtoMsg;
void RemoveGroupMembersHandler(const muduo::net::TcpConnectionPtr& conn,
                                const MyProtoMsg& msg,
                                void* ctx);
