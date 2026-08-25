#include "CreateGroupHandler.h"
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include <muduo/base/Logging.h>
#include <vector>


void CreateGroupHandler(const muduo::net::TcpConnectionPtr& conn,
                        const MyProtoMsg& msg,
                        void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();

    MyProtoMsg response;
    response.head.server = msg.head.server;

    std::string token = msg.body["token"].asString();
    std::string GroupName = msg.body["group"].asString();
    std::string username = redis.get("token:"+token);

    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "your token has expired , please Log in again";
        LOG_INFO << "CreateGroupHandler : token has expired ! - " << username;
    }
    else if(redis.exists("groupname:" + GroupName))
    {
        response.body["status"] = "error";
        response.body["msg"] = "群名重复了,请重新命名!";
        LOG_INFO << "CreateGroupHandler : GroupName has existed ! - " << username;
    }
    else
    {
        const Json::Value& members = msg.body["members"];
        if(members.size() < 2)
        {
            response.body["status"] = "error";
            response.body["msg"] = "建群至少需要3人(群主+2个初始成员)";
        }
        else
        {
            std::vector<std::string> ok;
            for(int i = 0; i < (int)members.size(); i++)
            {
                std::string m = members[i].asString();
                if(!redis.sismember("friends:" + username, m))
                {
                    response.body["status"] = "error";
                    response.body["msg"] = m + " 不是你的好友,建群失败";
                    ok.clear();
                    break;
                }
                ok.push_back(m);
            }
            if(!ok.empty())
            {
                std::string groupId = redis.incr("groups:counter");
                redis.set("groupname:" + GroupName , groupId);
                redis.hset("group:" + groupId , "owner" , username);
                redis.hset("group:" + groupId , "name" , GroupName);
                redis.sadd("group:" + groupId + ":members" , username);
                redis.sadd("group:" + groupId + ":admins" , username);
                redis.sadd("user:" + username + ":groups" , groupId);
                for(size_t i = 0; i < ok.size(); i++)
                {
                    redis.sadd("group:" + groupId + ":members" , ok[i]);
                    redis.sadd("user:" + ok[i] + ":groups" , groupId);
                }
                response.body["status"] = "ok";
                response.body["msg"] = "创建群聊成功! 已将您的身份设置为群主";
                response.body["group"] = GroupName;
                LOG_INFO << "CreateGroupHandler: successful ! - " << username << "build - " << GroupName;
            }
        }
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn->send(data ,len);
    delete[] data;
}