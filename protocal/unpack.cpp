#include "unpack.h"
#include <muduo/net/Buffer.h>
#include <json/json.h>
#include "protocal.h"
int MyProtoDecode::decode(muduo::net::Buffer* buf, MyProtoMsg*& outMsg)
{
    if(buf->readableBytes() < MY_PROTO_HEAD_SIZE)
        return 0;
    uint8_t* headPtr = (uint8_t*)buf->peek();
    uint16_t server = *(uint16_t*)(headPtr);
    uint32_t msgLen = *(uint32_t*)(headPtr + 2);
    if(msgLen < MY_PROTO_HEAD_SIZE) return -1;   
    if(buf->readableBytes() < msgLen ) return 0;
    std::string bodyStr((char*)buf->peek() + MY_PROTO_HEAD_SIZE,
                        msgLen - MY_PROTO_HEAD_SIZE);
    Json::Reader  reader;
    MyProtoMsg* pMsg = new MyProtoMsg();
    reader.parse(bodyStr,pMsg->body);     //如果用std::vector<Json::Value>等去写很麻烦
    pMsg->head.server = server;
    pMsg->head.len = msgLen;
    outMsg = pMsg;
    buf->retrieve(msgLen);
    return 1;
}