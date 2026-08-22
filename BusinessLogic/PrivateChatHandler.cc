#include "PrivateChatHandler.h"
#include <muduo/base/Logging.h>
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include <string>
#include <cstring>
#include <string.h>
#include "AIclient.h"
#include "MySqlClient.h"
void PrivateChatHandler(const muduo::net::TcpConnectionPtr& conn,
                        const MyProtoMsg& msg,
                        void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server -> redis();
    MyProtoMsg response;
    response.head.server = msg.head.server;
    std::string token = msg.body["token"].asString();
    std::string FriendName = msg.body["to"].asString();
    std::string username = redis.get("token:"+token);

    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "token过期 , 请重新登录";
        LOG_INFO << "PrivateChatHandler:token has expired ! - " << username;
    }
    else if(FriendName != "AI_bot" && !redis.sismember("friends:"+username , FriendName))
    {
        response.body["status"] = "error";
        response.body["msg"] = "抱歉,您和他/她还不是好友";
        LOG_INFO << "PrivateChatHandler:cann't pass the friends' verification ! - " << username;
    }
    else if(redis.sismember("blocked:"+ FriendName , username))
    {
        response.body["status"] = "error";
        response.body["msg"] = "抱歉，您被对方拉黑了，无法私聊";
        LOG_INFO << "PrivateChatHandler:user has been blocked! - " << username;
    }    
    else
    {
        if(FriendName == "AI_bot")
        {
            std::string userMsg = msg.body["msg"].asString();
            std::string reply = server->ai().chat(userMsg);
            response.body["status"] = "ok";
            response.body["msg"] = reply;
            server->mysql().SaveHistory(msg.body, username  , FriendName,"0");
        }
        else
        {
            auto Friendconn = server->GetconnByUser(FriendName);
            if(Friendconn)
            {
                const_cast<MyProtoMsg&>(msg).body["from"] = username;
                MyProtoEncode encoder;
                uint32_t len = 0;
                uint8_t* data = encoder.encode(const_cast<MyProtoMsg*>(&msg),len);
                Friendconn -> send(data,len);
                delete[] data;
            }
            else
            {
                const_cast<MyProtoMsg&>(msg).body["from"]=username;
                server->mysql().SaveOffline(msg.body,username,FriendName, "0");
            }
            response.body["status"] = "ok";
            response.body["msg"] = "message已发送"; 
            server->mysql().SaveHistory(msg.body ,username,FriendName,"0");
            LOG_INFO << "PrivateChatHandler: - " << username << "to - " << FriendName; 
        }
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn->send(data,len);
    delete[] data;     
}
