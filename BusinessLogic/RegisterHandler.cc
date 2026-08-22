#include "RegisterHandler.h"
#include <muduo/base/Logging.h>
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include "Hashtools.h"
void HandleRegister(const muduo::net::TcpConnectionPtr& conn,
                    const MyProtoMsg& msg,
                    void* ctx)
{
    std::string username = msg.body["username"].asString();
    std::string password = msg.body["password"].asString();
    LOG_INFO << "handleRegister: username=" << username;

    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();

    MyProtoMsg response;
    response.head.server = msg.head.server;
    std::string userKey = "user:" + username;
    std::string email = msg.body["email"].asString();
    std::string code = msg.body["code"].asString();
    std::string storedCode = redis.get("verify_code:" + email);
    if(storedCode.empty() || storedCode != code)
    {
        response.body["status"] = "error";
        response.body["msg"] = "验证码错误或已过期";
    }
    else if(redis.hexists(userKey,"password"))
    {
        response.body["status"] = "error";
        response.body["msg"] = "用户名已存在,请重试";
        LOG_INFO << "RegisterHandler: username already exists! - " << username;

    }
    else
    {
        std::string salt = generateSalt();
        redis.hset(userKey,"salt",salt);
        std::string HashWord = sha256(password , salt);
        redis.hset(userKey,"password",HashWord);
        redis.hset(userKey,"email",email);
        response.body["status"] = "ok";
        response.body["msg"] = "注册成功";
        response.body["username"] = username;
        LOG_INFO << "RegisterHandler: RegisterHandle - successfully - " << username; 
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn -> send(data , len);
    delete[] data;
}
    
