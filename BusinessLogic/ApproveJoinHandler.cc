#include "ApproveJoinHandler.h"
#include <muduo/base/Logging.h>
#include "../protocal/pack.h"
#include "../protocal/protocal.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
void ApproveJoinHandler(const muduo::net::TcpConnectionPtr& conn,
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
        response.body["msg"] = "Sorry,检测到您token过期,请您重新登录！";
        LOG_INFO << "ApproveJoinHandler: token has expired !";
    }
    else if(redis.hget("group:"+groupid , "owner") != username && !redis.sismember("group:" + groupid + ":admins" , username))
    {
        response.body["status"] = "error";
        response.body["msg"] = "你不是群主或管理员!";
        LOG_INFO << "user isn't owner or admin ! - " << username;
    }
    else if(!redis.sismember("group:" + groupid +":pending" , target))
    {
        response.body["status"] = "error";
        response.body["msg"] = "未检测到该用户申请加入群聊";
        LOG_INFO << "ApproveJoinHandler:the user does't apply to Joining the group ! - " << target << ":group:" << groupname; 
     }
     else
     {
           redis.sadd("group:" + groupid + ":members" , target);
           redis.sadd("user:" + target + ":groups" , groupid);
           redis.srem("group:" + groupid + ":pending" , target);
            auto targetconn = server->GetconnByUser(target);
            if(targetconn)
            {
                MyProtoMsg notification;
                notification.head.server = 911;
                notification.body["from"] = username;
                notification.body["msg"] = "你已加入群：" + groupname;
                MyProtoEncode encoder;
                uint32_t len = 0;
                uint8_t* data = encoder.encode(&notification,len);
              targetconn->send(data,len);
                delete[] data;
            } 
           response.body["status"] = "ok";
           response.body["msg"] = "成功加入该群聊";
           LOG_INFO << "ApproveJoinHandler: success ! - " << target << "join the group - " << groupname;
     } 
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response , len);
    conn->send(data , len);
    delete[] data;

}