#include "LeaveGroupHandler.h"
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include <muduo/base/Logging.h>
void LeaveGroupHandler(const muduo::net::TcpConnectionPtr& conn,
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
        response.body["msg"] = "token已过期,请重新登录";
        LOG_INFO << "LeaveGroupHandler:token has expired - " << username;
    }
    else if(!redis.exists("group:"+groupid))
    {
        response.body["status"] = "error";
        response.body["msg"] = "抱歉，此群不存在";
        LOG_INFO << "LeaveGroupHandler:the group doesn't exist ! - " << username;
    }
    else if(!redis.sismember("group:" + groupid + ":members" , username))
    {
        response.body["status"] = "error";
        response.body["msg"] = "抱歉,您不是群成员,无法退群!";
        LOG_INFO << "LeaveGroupHandler:the user isn't in the group! - " << username;
    }else if(redis.hget("group:" + groupid , "owner") == username)
    {
        response.body["status"] = "error";
        response.body["msg"] = "您是群主,不能直接退群,请先解散群";
        LOG_INFO << "LeaveGroupHandler:owner can't leave! - " << username;
    }
    else
    {
        redis.srem("group:" + groupid + ":members" , username);
        redis.srem("group:" + groupid + ":admins" , username);
        redis.srem("user:" + username + ":groups" , groupid);
        std::string owner = redis.hget("group:" + groupid,"owner");
        auto ownerconn = server->GetconnByUser(owner);
        if(ownerconn)
        {
            MyProtoMsg notification;
            notification.head.server = 911;
            notification.body["from"] = username;
            notification.body["msg"] = "退出了该群聊：" + groupname;
            MyProtoEncode encoder;
            uint32_t len = 0;
            uint8_t* data = encoder.encode(&notification,len);
            ownerconn->send(data,len);
            delete[] data;
        }
        response.body["status"] = "ok";
        response.body["msg"] = "成功退出群聊";
        LOG_INFO << "LeaveGroupHandler:success ! - " << username << "group:" << groupname;
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn->send(data,len);
    delete[] data;
}