#include "AddFriendHandler.h"
#include <muduo/base/Logging.h>
#include "../protocal/pack.h"
#include "../protocal/protocal.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
void AddFriendHandler(const muduo::net::TcpConnectionPtr& conn,
                      const MyProtoMsg& msg,
                      void* ctx)    
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server -> redis();

    MyProtoMsg response;
    response.head.server = msg.head.server;
    std::string token = msg.body["token"].asString();
    std::string FriendName = msg.body["friend"].asString();
    std::string username = redis.get("token:"+token);
    
    
    if(username.empty())
    {
        response.body["status"] = "error"; 
        response.body["msg"] = "Sorry,检测到您token过期,请您重新登录！";
        LOG_INFO << "AddFriendHandler: The token has expired! - "<< username 
        << "Sorry,Add - " << FriendName << "failure!";
    }
    else if(username == FriendName)
    {
        response.body["status"] = "error";
        response.body["msg"] = "不能添加自己为好友";
        LOG_INFO << "AddFriendHandler: can't add yourself - " << username;
    }
    else 
    {
        redis.sadd("friend_request:"+ FriendName, username);
        auto friendconn = server->GetconnByUser(FriendName);
        if(friendconn)
         {
             MyProtoMsg notification;
            notification.head.server = 911;
            notification.body["from"] = username;
            notification.body["msg"] = "请求加你为好友";
            MyProtoEncode encoder;
            uint32_t len = 0;
            uint8_t* data = encoder.encode(&notification,len);
            friendconn->send(data,len);
            delete[] data;
        }
        response.body["status"] = "ok";
        response.body["msg"] = "好友申请以发送,等待确认ing";
        LOG_INFO << " AddFriendHandler's application successfully ! - " << username;
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn->send(data,len);
    delete[] data;


}