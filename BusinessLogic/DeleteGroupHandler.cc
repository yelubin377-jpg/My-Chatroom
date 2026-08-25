#include "DeleteGroupHandler.h"
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include <muduo/base/Logging.h>
void DeleteGroupHandler(const muduo::net::TcpConnectionPtr& conn,
                        const MyProtoMsg& msg,
                        void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();

    MyProtoMsg response;
    response.head.server = msg.head.server;
    
    std::string token = msg.body["token"].asString();
    std::string username = redis.get("token:"+token);
     std::string groupname = msg.body["group"].asString();
    std::string GroupId = server->NameToId(groupname);


    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "token过期,请重新登录";
        LOG_INFO << "DeleteGroupHandler:token has expired! - " << username;
    }
    else if(redis.hget("group:" + GroupId , "owner") != username)
    {
        response.body["status"] = "error";
        response.body["msg"] = "您不是群主，无法解散群聊";
        LOG_INFO << "DeleteGroupHandler:user isn't owner ! - " << username; 
    } 
    else
    {
        std::vector<std::string> members = redis.smembers("group:" + GroupId + ":members");
        for(size_t i = 0;i < members.size();i++)
        {
            redis.srem("user:" + members[i] + ":groups" , GroupId);
            auto memberconn = server->GetconnByUser(members[i]);
            if(memberconn)
            {
                MyProtoMsg notification;
                notification.head.server = 911;
                notification.body["from"] = username;
                notification.body["msg"] = "群已解散：" + groupname;
                MyProtoEncode encoder;
                uint32_t len = 0;
                uint8_t* data = encoder.encode(&notification,len);
                memberconn->send(data,len);
                delete[] data;
            }
        }
        std::string name = redis.hget("group:" + GroupId,"name");
        redis.del("group:"+GroupId);
        redis.del("group:"+GroupId + ":members");
        redis.del("group:"+ GroupId + ":admins");
        redis.del("group:"+ GroupId + ":pending");
        redis.del("groupname:"+ name);
        response.body["status"] = "ok";
        response.body["msg"] = "群聊解散成功";
        LOG_INFO << "DeleteGroupHandler: success ! - " << username << ":groupname:"<< groupname << "has already disbanded !";

    }
    MyProtoEncode encoder;
    uint32_t len = 0;   
    uint8_t* data = encoder.encode(&response , len);
    conn -> send(data,len);
    delete[] data;

}