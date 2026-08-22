#include "QueryHistoryHandler.h"
#include <string>
#include <vector>
#include "../server/ChatServer.h"
#include "redisClient.h"
#include <json/json.h>
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include <muduo/base/Logging.h>

void QueryHistoryHandler(const muduo::net::TcpConnectionPtr& conn,
                              const MyProtoMsg& msg,
                              void* ctx)
{
      ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();
     MyProtoMsg response;
    response.head.server = msg.head.server;
   int count = msg.body.get("count",300).asInt();
    std::string token = msg.body["token"].asString();
    std::string username = redis.get("token:" + token);
     std::string groupname = msg.body["group"].asString();
    std::string Friendname = msg.body["friend"].asString();
    
    typedef std::vector<std::string> ves;
    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "token已过期, 请重新登录！";
        LOG_INFO << "QueryHIstoryHandler:token has expired - "  << username;
    }else if(groupname.empty()) //查私
    {
        std::vector<Json::Value> getGPH = server->mysql().GPH(username,Friendname,count); //后续补长度，不小心写死300；
        Json::Value result;
        for(int i = 0; i < getGPH.size() ; i++)
        {
            result.append(getGPH[i]);   //串串儿
        }
        response.body["status"] = "ok";
        response.body["msg"] = result.empty() ? "没有历史记录" : "查询成功 ! ";
        response.body["history"] = result;
    }
    else
    {
         std::string groupid = server->NameToId(groupname);
        std::vector<Json::Value> getGGH = server->mysql().GGH(username,groupid,count); 
        Json::Value result;
        for(int i = 0; i < getGGH.size() ; i++)
        {
            result.append(getGGH[i]); 
        }
        response.body["status"] = "ok";
        response.body["msg"] =result.empty() ? "没有历史记录" : "查询成功 ! ";
        response.body["history"] = result;
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn->send(data,len);
    delete[] data;
}