#include "LoginByCodeHandler.h"
#include "../protocal/pack.h"
#include "../protocal/protocal.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include <random>
void LoginByCodeHandler(const muduo::net::TcpConnectionPtr& conn,
                        const MyProtoMsg& msg, void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();

    MyProtoMsg response;
    response.head.server = msg.head.server;
    std::string username = msg.body["username"].asString();
    std::string code = msg.body["code"].asString();

    std::string userKey = "user:" + username;
    std::string email = redis.hget(userKey, "email");
    std::string storedCode = redis.get("verify_code:" + email);

    if(email.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "该账号未绑定邮箱";
    }
    else if(storedCode.empty() || storedCode != code)
    {
        response.body["status"] = "error";
        response.body["msg"] = "验证码错误或已过期";
    }
    else
    {
        std::random_device rd;
        std::string token = std::to_string(rd()) + std::to_string(rd()) + std::to_string(rd());
        redis.set("token:" + token, username);
        redis.expire("token:" + token, 86400);
        server->AddOnlineUser(username, conn);
        redis.del("verify_code:" + email);
        response.body["status"] = "ok";
        response.body["msg"] = "登录成功";
        response.body["token"] = token;
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response, len);
    conn->send(data, len);
    delete[] data;
}