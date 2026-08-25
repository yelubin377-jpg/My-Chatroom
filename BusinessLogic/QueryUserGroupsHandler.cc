#include "QueryUserGroupsHandler.h"
#include <string.h>
#include <string>
#include <cstring>
#include <vector>
#include "../server/ChatServer.h"
#include "redisClient.h"
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include <muduo/base/Logging.h>
void QueryUserGroupsHandler(const muduo::net::TcpConnectionPtr& conn,
                              const MyProtoMsg& msg,
                              void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();
    MyProtoMsg response;
    response.head.server = msg.head.server;

    std::string token = msg.body["token"].asString();
    std::string username = redis.get("token:" + token);
    
    typedef std::vector<std::string> ves;
    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "token已过期, 请重新登录！";
        LOG_INFO << "QueryUserGroupsHandler:token has expired - "  << username;
    }
    else
    {
        ves groupid = redis.smembers("user:"+username+":groups");
        response.body["groups"] = Json::Value(Json::arrayValue);
        for(size_t i = 0;i < groupid.size() ; i++)
        {
            std::string name = redis.hget("group:" + groupid[i] , "name"); 
            response.body["groups"][(int)i] = name;
        }
        response.body["status"] = "ok";
        response.body["msg"] = "成功取出用户加入的群聊名单";
        LOG_INFO << "QueryUserGroupHandler: success! - " << username;
     }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn->send(data,len);
    delete[] data;   
}