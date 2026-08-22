#include "pack.h"
#include "protocal.h"
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <json/json.h>
#include <map>

uint8_t* MyProtoEncode::encode(MyProtoMsg* pMsg,uint32_t& len)
{
    uint8_t* pData = NULL;
    Json::FastWriter fwriter; 


    std::string bodyStr = fwriter.write(pMsg->body);//序列化

    len = MY_PROTO_HEAD_SIZE + (uint32_t)bodyStr.size();
    pMsg->head.len = len;
    pData = new uint8_t[len];
    //编码协议头
    *(uint16_t*) pData = pMsg->head.server; //*pData不行，复盘的时候思考s
    *(uint32_t*)(pData +2)=pMsg->head.len; // 函数内部没有通过二级指针修改pData的数据，修改的是临时数据
    //打包协议体
    memcpy(pData + MY_PROTO_HEAD_SIZE,bodyStr.c_str(),bodyStr.size());
    return pData; //返回消息首部地址
}

