#pragma once
#ifndef MY_PACK_H
#define MY_PACK_H
#include "protocal.h"
#include<stdint.h>
class MyProtoEncode // 协议封装类
{
public:
    uint8_t* encode(MyProtoMsg* pMsg,uint32_t& len);//传入的消息体封装函数
    //传入的pMsg里面只有部分数据，比如Json协议体，服务号，我们对消息编码后会修改长度信息，这时需要重新编码协议
    //encode返回长度信息，用于后面socket发送数据
};
#endif