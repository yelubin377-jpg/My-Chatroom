
#include "DeleteAccountHandler.h"
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include <muduo/base/Logging.h>

void DeleteAccountHandler(const muduo::net::TcpConnectionPtr& conn,
                          const MyProtoMsg& msg,
                          void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();
    MyProtoMsg response;
    response.head.server = msg.head.server;
    std::string token = msg.body["token"].asString();
    std::string username = redis.get("token:" + token);

    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "token过期,请重新登录";
        LOG_INFO << "DeleteAccountHandler:token expired - " << token;
    }
    else
    {
        std::vector<std::string> friends = redis.smembers("friends:" + username);
        for(size_t i = 0; i < friends.size(); i++)
        {
            redis.srem("friends:" + friends[i], username);
        }
        std::vector<std::string> groups = redis.smembers("user:" + username + ":groups");
        for(size_t i = 0; i < groups.size(); i++)
        {
            redis.srem("group:" + groups[i] + ":members", username);
            redis.srem("group:" + groups[i] + ":admins", username);
        }
        // 删掉用户自己的所有数据
        redis.del("user:" + username);
        redis.del("salt:" + username);
        redis.del("friends:" + username);
        redis.del("blocked:" + username);
        redis.del("user:" + username + ":groups");
        redis.del("token:" + token);
         response.body["status"] = "ok";
        response.body["msg"] = "账号已注销";
        LOG_INFO << "DeleteAccountHandler: success - " << username;
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response, len);
    conn->send(data, len);
    delete[] data;
}