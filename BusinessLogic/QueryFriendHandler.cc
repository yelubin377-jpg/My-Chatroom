#include "QueryFriendHandler.h"
#include <muduo/base/Logging.h>
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include <string>
#include <cstring>
#include <string.h>
void QueryFriendHandler(const muduo::net::TcpConnectionPtr& conn,
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
        response.body["msg"] = " token已过期,请重新登录! ";
        LOG_INFO << "QueryFriendHandler: token has expired! - "<<username; 
    }
    else
    {
        std::vector<std::string> FriendsLine = redis.smembers("friends:"+ username);
        //
        Json::Value FriendsList(Json::arrayValue);
        for(const std::string& f : FriendsLine)
        {
            std::string tag = server->TrueOnline(f) ? " [在线]" : " [离线]";
            FriendsList.append(f + tag);
        }
        response.body["status"] = "ok";
        response.body["msg"] = "好友列表:";
        response.body["friends"] = FriendsList;

        LOG_INFO << "QueryFriendHandler: success ! - "<< username;
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn ->send(data,len);
    delete[] data;

}