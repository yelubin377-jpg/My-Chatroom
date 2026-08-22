#include "GroupChatHandler.h"
#include <muduo/base/Logging.h>
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include <vector>
#include <string.h>
void GroupChatHandler(const muduo::net::TcpConnectionPtr& conn,
                      const MyProtoMsg& msg,
                      void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server -> redis();
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
        response.body["msg"] = "token 已过期 , 请重新登录";
        LOG_INFO << "GroupChatHandler: token has expired! - " << username;
    }
    else if(!redis.sismember("group:"+ groupid + ":members" , username))
    {
        response.body["status"] = "error";
        response.body["msg"] = "抱歉，您不是群成员,无权限访问该群";
        LOG_INFO << "GroupChatHandler: user aren't in this group ! - " << username << ":group" << groupid;
    }
    else
    {
        ves members = redis.smembers("group:" + groupid + ":members");
        for(size_t i = 0;i < members.size(); i++)
        {
            if(members[i] == username) continue;
            else 
            {
                auto memberconn = server->GetconnByUser(members[i]);
                if(memberconn)
                {
                    MyProtoEncode encoder;
                    uint32_t len = 0;
                    uint8_t* data = encoder.encode(const_cast<MyProtoMsg*>(&msg) , len);
                    memberconn -> send(data,len);
                    delete[] data;
                }
                else
                {
                    const_cast<MyProtoMsg&>(msg).body["from"] = username;
                    server->mysql().SaveOffline(msg.body,  username,members[i],  groupid);
            }
            }
        }
        response.body["status"] = "ok";
        response.body["msg"] = "消息发送成功";
        server->mysql().SaveHistory(msg.body,username,"",groupid);
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn->send(data,len);
    delete[] data;

}
