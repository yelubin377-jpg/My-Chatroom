#include "JoinGroupHandler.h"
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include <muduo/base/Logging.h>
void JoinGroupHandler(const muduo::net::TcpConnectionPtr& conn,
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
    std::string groupid = server->NameToId(groupname);



    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "token已过期,请您重新登录";
        LOG_INFO << "JoinGroupHandler:token has expired - " << username;
    }
    else if(!redis.exists("group:"+groupid))
    {
        response.body["status"] = "error";
        response.body["msg"] = "抱歉,这个群不存在";
        LOG_INFO << "JoinGroupHandler:the group doesn't exist! - " << username << ":" << groupname;
    }
    else if(redis.sismember("group:" + groupid + ":members" , username))
    {
        response.body["status"] = "error";
        response.body["msg"] = "您已在此群,请勿重复加群";
        LOG_INFO << "JoinGroupHandler: user already in group ! - " << username << ":group - " << groupname;
    }
    else if(redis.sismember("group:" + groupid + ":pending" , username))
    {
        response.body["status"] = "error";
        response.body["msg"] = "你已经提交过申请，请勿重复提交！等待管理员批准! ";
        LOG_INFO << "JoinGroupHandler: already in pending - " << username;
    }
    else
    {
        redis.sadd("group:"+groupid + ":pending",username);
        std::string owner = redis.hget("group:"+groupid,"owner");
        auto ownerconn = server->GetconnByUser(owner);
        if(ownerconn)
        {
            MyProtoMsg notification;
            notification.head.server = 911;
            notification.body["from"] = username;
            notification.body["msg"] = "申请加入群： " + groupname;
            MyProtoEncode encoder;
            uint32_t len = 0;
            uint8_t* data = encoder.encode(&notification,len);
            ownerconn->send(data,len);
            delete[] data;
        }
        response.body["status"] = "ok";
        response.body["msg"] = "成功提交加入此群聊的申请： - " + groupname + ":user: " + username;
        LOG_INFO << "JoinGroupHandler: apply success ! - " << username << ":group" << groupid; 
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn->send(data , len);
    delete[] data;
}