#pragma once 
#include <stdint.h> //uint
#include <json/json.h>
const uint32_t MY_PROTO_HEAD_SIZE = 6;  
const uint32_t MY_PROTO_MAX_SIZE = 10*1024*1024; 
class MyProtoHead 
{
public: 
    uint16_t server;//工作函数key - MessageType
    uint32_t len;
};

class MyProtoMsg 
{
public:
    MyProtoHead head; 
    Json::Value body; 
};

