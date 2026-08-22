#include "GetVerifyCodeHandler.h"
#include <muduo/base/Logging.h>
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include <random>

void GetVerifyCodeHandler(const muduo::net::TcpConnectionPtr& conn,
                          const MyProtoMsg& msg,
                          void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server -> redis();
    std::string email = msg.body["email"].asString();

    std::random_device rd;
    std::string code;
    for(int i = 0; i < 6; i++)
    {
        code += std::to_string(rd() % 10);
    }

    redis.set("verify_code:" + email, code);
    redis.expire("verify_code:" + email, 300);

    bool ok = server->email().send(email, "验证码", "你的验证码是: " + code);

    MyProtoMsg response;
    response.head.server = msg.head.server;
    response.body["status"] = ok ? "ok" : "error";
    response.body["msg"] = ok ? "验证码已发送" : "发送失败";

    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response, len);
    conn->send(data, len);
    delete[] data;
}