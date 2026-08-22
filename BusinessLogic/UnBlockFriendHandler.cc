#include "UnBlockFriendHandler.h"
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include <muduo/base/Logging.h>

void UnBlockFriendHandler(const muduo::net::TcpConnectionPtr& conn,
                          const MyProtoMsg& msg,
                          void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();
    MyProtoMsg response;
    response.head.server = msg.head.server;
    std::string token = msg.body["token"].asString();
    std::string friendName = msg.body["friend"].asString();
    std::string username = redis.get("token:"+token);

    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "抱歉,您的token已过期,请重新登录";
        LOG_INFO << "UnBlockFriendHandler: token has expired! - " << username;
    }
    else
    {
        redis.srem("blocked:"+username , friendName);
        response.body["status"] = "ok";
        response.body["msg"] = "取消屏蔽成功";
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn->send(data,len);
    delete[] data;
}

