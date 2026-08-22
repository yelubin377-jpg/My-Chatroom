#include "AcceptFriendHandler.h"
#include "../protocal/pack.h"
#include "../protocal/protocal.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
void AcceptFriendHandler(const muduo::net::TcpConnectionPtr& conn,
                         const MyProtoMsg& msg,
                         void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();

    MyProtoMsg response;
    response.head.server = msg.head.server;
    std::string username = redis.get("token:" + msg.body["token"].asString());
    std::string FriendName = msg.body["friend"].asString();

    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "token过期, 请重新登录";
    }
    else if(!redis.sismember("friend_request:" + username, FriendName))
    {
        response.body["status"] = "error";
        response.body["msg"] = "没有这个好友申请";
    }
    else
    {
        redis.sadd("friends:" + username, FriendName);
        redis.sadd("friends:" + FriendName, username);
        redis.srem("friend_request:" + username, FriendName);
        response.body["status"] = "ok";
        response.body["msg"] = "已同意好友申请";
        
        auto yConn = server->GetconnByUser(FriendName);
        if(yConn)
        {
            MyProtoMsg noti;
            noti.head.server = 911;
            noti.body["from"] = username;
            noti.body["msg"] = "同意了你的好友申请";
            MyProtoEncode enc;
            uint32_t len = 0;
            uint8_t* data = enc.encode(&noti, len);
            yConn->send(data, len);
            delete[] data;
        }
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response, len);
    conn->send(data, len);
    delete[] data;
}