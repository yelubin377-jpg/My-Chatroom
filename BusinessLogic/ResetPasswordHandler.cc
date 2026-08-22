#include "ResetPasswordHandler.h"
#include "../protocal/pack.h"
#include "../protocal/protocal.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include "Hashtools.h"

void ResetPasswordHandler(const muduo::net::TcpConnectionPtr& conn,
                          const MyProtoMsg& msg,
                          void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();

    MyProtoMsg response;
    response.head.server = msg.head.server;
    std::string username = msg.body["username"].asString();
    std::string email = msg.body["email"].asString();
    std::string code = msg.body["code"].asString();
    std::string newpass = msg.body["newpass"].asString();

    std::string userKey = "user:" + username;
    std::string storedCode = redis.get("verify_code:" + email);
    std::string storedEmail = redis.hget(userKey, "email");

    if(storedCode.empty() || storedCode != code)
    {
        response.body["status"] = "error";
        response.body["msg"] = "验证码错误或已过期";
    }
    else if(storedEmail != email)
    {
        response.body["status"] = "error";
        response.body["msg"] = "邮箱与账号不匹配";
    }
    else
    {
        std::string salt = redis.hget(userKey, "salt");
        std::string newHash = sha256(newpass, salt);
        redis.hset(userKey, "password", newHash);
        response.body["status"] = "ok";
        response.body["msg"] = "密码重置成功";
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response, len);
    conn->send(data, len);
    delete[] data;
}