#include "LogoutHandler.h"
#include <muduo/base/Logging.h>
#include "../protocal/pack.h"
#include "../protocal/protocal.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
void LogoutHandler(const muduo::net::TcpConnectionPtr& conn,
                   const MyProtoMsg& msg,
                   void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();
    
    MyProtoMsg response;
    response.head.server = msg.head.server;
    std::string token = msg.body["token"].asString();
    
    redis.del("token:"+token);
    response.body["status"] = "ok";
    response.body["msg"] = "安全退出";
    LOG_INFO << " LogoutHandler: success - " << token;

    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn->send(data,len);
    delete[] data;
}