#include "LoginHandler.h"
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include <muduo/base/Logging.h>
#include "redisClient.h" 
#include <random>
#include <vector>
#include "Hashtools.h"
void HandleLogin(const muduo::net::TcpConnectionPtr& conn,
                 const MyProtoMsg& msg,
                 void* ctx)
{
    std::string username = msg.body["username"].asString();
    std::string password = msg.body["password"].asString();
    int judger = 0;
    LOG_INFO << "handleLogin: username=" << username;

    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();
    
    MyProtoMsg response;
    response.head.server = msg.head.server;
    std::string userKey = "user:" + username;

    if(!redis.hexists(userKey,"password")) // key+field，看哈希表有没有
    {
        response.body["status"] = "error";
        response.body["msg"] = "用户不存在";
        LOG_INFO << "HandleLogin: user not found - " << username;
    }
    else
    {
        std::string salt = redis.hget(userKey,"salt");
        std::string WriteWord = sha256(password , salt);//补全的，要比对的
        std::string StorePassword = redis.hget(userKey,"password"); //正确密码

        if(WriteWord != StorePassword)
        {
            response.body["status"] = "error";
            response.body["msg"] = "密码错误,请重新登录";
            LOG_INFO << "LoginHandler: wrong password - " << username;
        }
        else
        {   
            
            std::random_device rd;
            std::string token = std::to_string(rd()) + std::to_string(rd()) + std::to_string(rd());
           redis.set("token:" + token,username); //依旧k + f
            redis.expire("token:" + token , 86400);//k+ 秒
            judger = 1;
            response.body["status"] = "ok";
            response.body["msg"] = "登录成功！";
            response.body["token"] = token;
            LOG_INFO << "LoginHandler: success perfectly - "<< username << "- token = " << token;
            server -> AddOnlineUser(username,conn);
            
        }   
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response,len);
    conn->send(data,len);
    delete[] data;
    if(judger)
    {
         std::vector<Json::Value> offline = server->mysql().PushOffline(username);
        for(int i = 0; i < offline.size(); i++)
        {
            MyProtoMsg push;
            push.head.server = (offline[i]["Group"] == "0") ? 729 : 06;
            push.body["from"] = offline[i]["User"];
            push.body["msg"] = offline[i]["msg"];
            push.body["group_id"] = offline[i]["Group"];
            MyProtoEncode pushEncoder;
            uint32_t pushLen = 0;
            uint8_t* pushData = pushEncoder.encode(&push,pushLen);
            conn->send(pushData,pushLen);
            delete[] pushData;
        }
        server->mysql().ClearOffline(username);
        std::vector<std::string> requests = redis.smembers("friend_request:" + username);
        for(const std::string& requester : requests)
        {
            MyProtoMsg noti;
            noti.head.server = 911;
            noti.body["from"] = requester;
            noti.body["msg"] = "申请加你为好友,请输入 /yes或者 /no";
            MyProtoEncode enc;
            uint32_t len = 0;
            uint8_t* data = enc.encode(&noti, len);
            conn->send(data, len);
            delete[] data;
        }
    }}