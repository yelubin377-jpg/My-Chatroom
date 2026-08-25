#include "DeleteFriendHandler.h"
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include <muduo/base/Logging.h>
#include "../server/ChatServer.h"
#include "redisClient.h"

void DeleteFriendHandler(const muduo::net::TcpConnectionPtr& conn,
                         const MyProtoMsg& msg,
                         void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();

    MyProtoMsg response;
    response.head.server = msg.head.server;
    std::string token = msg.body["token"].asString();
    std::string friendname = msg.body["friend"].asString();
    std::string username = redis.get("token:"+token);

    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "Sorry,token过期,删除好友失败，请重新登录";
        LOG_INFO << "DeleteFriendHandler: token has expired - "<< username;
    }
    else if(!redis.sismember("friends:" + username, friendname))
    {
        response.body["status"] = "error";
        response.body["msg"] = "你们还不是好友,无法删除";
    }
    else
    {
        redis.srem("friends:"+username,friendname);
        redis.srem("friends:"+friendname,username);
        auto friendconn = server->GetconnByUser(friendname);
        if(friendconn)
        {
            MyProtoMsg notification;
            notification.head.server = 911;
            notification.body["from"] = username;
            notification.body["msg"] = "删除了你";
            MyProtoEncode encoder;
            uint32_t len = 0;
            uint8_t* data = encoder.encode(&notification,len);
            friendconn->send(data,len);
            delete[] data;
        }
        
        response.body["status"] = "ok";
        response.body["msg"] = "成功删除好友";   
        LOG_INFO << "DeleteFriendHandler:delete successfully - " << username;
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn->send(data,len);
    delete[] data;


}