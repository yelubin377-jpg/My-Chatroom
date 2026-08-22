#pragma once
#include <muduo/net/TcpConnection.h>
#include "../protocal/protocal.h"

class MyProtoMsg;
void DeleteAccountHandler(const muduo::net::TcpConnectionPtr& conn,
                          const MyProtoMsg& msg,
                          void* ctx);