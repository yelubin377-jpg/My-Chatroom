#include "RemoveGroupAdminHandler.h"
#include <muduo/base/Logging.h>
#include "../protocal/pack.h"
#include "../protocal/protocal.h"
#include "../server/ChatServer.h"
#include "redisClient.h"

void RemoveGroupAdminHandler(const muduo::net::TcpConnectionPtr& conn,
                              const MyProtoMsg& msg,
                              void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();
    MyProtoMsg response;
    response.head.server = msg.head.server;

    std::string token = msg.body["token"].asString();
    std::string username = redis.get("token:" + token);
    std::string target = msg.body["target"].asString();
    std::string groupname = msg.body["group"].asString();
    std::string groupid = server->NameToId(groupname);
    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "token已过期,请重新登录！";
        LOG_INFO << "RemoveGroupAdminHandler:token has expired ! - " << username;
    }
    else if(redis.hget("group:"+groupid,"owner") != username)
    {
        response.body["status"] = "error";
        response.body["msg"] = "抱歉，您不是群主，只有群主才能移除管理员" ;
        LOG_INFO << "RemoveGroupAdminHandler:user isn't owner! - " << username;
    }
    else if(!redis.sismember("group:"+groupid + ":admins", target))
    {
        response.body["status"] = "error";
        response.body["msg"] = "抱歉，该用户不是管理员,无法移除";
        LOG_INFO << "RemoveGroupAdminHandler:user isn't the admin ! - "<<target << ":" <<groupname;
    }
    else if(redis.hget("group:" + groupid , "owner") == target)
    {
        response.body["status"] = "error";
        response.body["msg"] = "不能移除群主";
        LOG_INFO << "RemoveGroupMembersHandler:can't remove owner - " << target;
    }
    else
    {
        redis.srem("group:" + groupid + ":admins" , target);
        auto targetconn = server->GetconnByUser(target);
         if(targetconn)
        {
            MyProtoMsg notification;
            notification.head.server = 911;
            notification.body["from"] = username;
            notification.body["msg"] = "你已被移除管理员：" + groupname;
            MyProtoEncode encoder;
            uint32_t len = 0;
            uint8_t* data = encoder.encode(&notification,len);
            targetconn->send(data,len);
            delete[] data;
        }
        response.body["status"] = "ok";
        response.body["msg"] = "remove - " + target + "'s admin position successfully ! ";
        LOG_INFO << "RemoveGroupAdminHandler: owner - " << username << "delete member - " << target << "'s Admin position of group - " << groupname;
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn -> send(data , len);
    delete[] data; 
}