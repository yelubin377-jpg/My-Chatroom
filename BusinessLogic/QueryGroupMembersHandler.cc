#include "QueryGroupMembersHandler.h"
#include <string.h>
#include <string>
#include <cstring>
#include <vector>
#include "../server/ChatServer.h"
#include "redisClient.h"
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include <muduo/base/Logging.h>
void QueryGroupMembersHandler(const muduo::net::TcpConnectionPtr& conn,
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

    
    typedef std::vector<std::string> ves;
    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "token已过期, 请重新登录！";
        LOG_INFO << "QueryGroupMembersHandler:token has expired - "  << username;
    }
    else if(!redis.exists("group:" + groupid))
    {
        response.body["status"] = "error";
        response.body["msg"] = "抱歉，你查询的群不存在!";
        LOG_INFO << "QueryGroupMembersHandler: the group doesn't exist ! - " << username << ":" << groupname;
    }
    else if(!redis.sismember("group:" + groupid + ":members" , username))
    {
        response.body["status"] = "error";
        response.body["msg"] = "抱歉，您还不是群成员，无法调取";
        LOG_INFO << "QueryGroupMembersHandler:user isn't in the group! - " << username << ":" <<groupname;
    }
    else
    {
        ves members = redis.smembers("group:"+ groupid + ":members");
        for(size_t i = 0; i < members.size() ; i++)
        {
            response.body["members"][(int)i] = members[i];           
        }
        response.body["status"] = "ok";
        response.body["msg"] = "群成员列表获取成功";
        LOG_INFO << "QueryGroupMembersHandler: success ! - " << username << "'s : " << groupname << "group";
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn->send(data,len);
    delete[] data;
}