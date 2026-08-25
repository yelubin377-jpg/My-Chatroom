#include "BlockFriendHandler.h"
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include <muduo/base/Logging.h>

void BlockFriendHandler(const muduo::net::TcpConnectionPtr& conn,                                   
                        const MyProtoMsg& msg,
                        void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();

    MyProtoMsg response;
    response.head.server = msg.head.server;
    std::string token = msg.body["token"].asString();
    //down ？
    std::string friendname = msg.body["friend"].asString();
    std::string username = redis.get("token:"+token);

    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "token无效,请重新登录!";
        LOG_INFO << "BlockFriendHandler: token has expired! - " << username;
    }
    else if(!redis.hexists("user:" + friendname, "password"))
    {
        response.body["status"] = "error";
        response.body["msg"] = "该用户不存在,无法屏蔽";
    }
    else
    {
        redis.sadd("blocked:" + username , friendname);
        response.body["status"] = "ok";
        response.body["msg"] = "屏蔽好友成功";
        LOG_INFO << "BlockFriendHandler: success! - " << username << "block - " << friendname;
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn->send(data,len);
    delete[] data;
}