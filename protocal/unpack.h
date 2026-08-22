#pragma once
#include <muduo/net/Buffer.h>
#include "protocal.h"

class MyProtoDecode
{
public:
    int decode(muduo::net::Buffer* buf,MyProtoMsg*& outMsg);
};

