#include "AddGroupAdminHandler.h"
#include <muduo/base/Logging.h>
#include "../protocal/pack.h"
#include "../protocal/protocal.h"
#include "../server/ChatServer.h"
#include "redisClient.h"

void AddGroupAdminHandler(const muduo::net::TcpConnectionPtr& conn,
                            const MyProtoMsg& msg,
                              void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();
    MyProtoMsg response;
    response.head.server = msg.head.server;

    std::string token = msg.body["token"].asString();
    std::string username = redis.get("token:" + token);
    std::string groupname = msg.body["group"].asString();
    std::string groupid = server->NameToId(groupname);
    std::string target = msg.body["target"].asString();
    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "token失效,请重新登录！";
        LOG_INFO << "AddGroupAdminHandler:token has expired ! - " << username;
    }
    else if(redis.hget("group:"+groupid,"owner") != username)
    {
        response.body["status"] = "error";
        response.body["msg"] = "只有群主才能设置管理员!" ;
        LOG_INFO << "AddGroupAdminHandler:user isn't owner! - " << username;
    }
    else if(!redis.sismember("group:"+groupid + ":members", target))
    {
        response.body["status"] = "error";
        response.body["msg"] = "该用户不是群成员,无法设置管理员";
        LOG_INFO << "AddGroupAdminHandler:user isn't in the group ! - "<<username << ":" <<groupname;
    }
    else if(redis.sismember("group:" + groupid + ":admins" , target))
    {
        response.body["status"] = "error";
        response.body["msg"] = "你已经是管理员,禁止重复设置";
        LOG_INFO << "AddGroupAdminHandler: the target has already been admin ! - " << target;
    }
    else
    {
        redis.sadd("group:" + groupid + ":admins" , target);
        auto targetconn = server->GetconnByUser(target);
        if(targetconn)
        {
            MyProtoMsg notification;
            notification.head.server = 911;
            notification.body["from"] = username;
            notification.body["msg"] = "你已成为管理员：" + groupname;
            MyProtoEncode encoder;
            uint32_t len = 0;
            uint8_t* data = encoder.encode(&notification,len);
            targetconn->send(data,len);
            delete[] data;
        }
        response.body["status"] = "ok";
        response.body["msg"] = target + " has already been admin !";
        LOG_INFO << "AddGroupAdminHandler: owner - " << username << "set member - " << target << "as the Admin of group - " << groupname;
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn -> send(data , len);
    delete[] data; 
}