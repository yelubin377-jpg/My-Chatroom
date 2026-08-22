#include "FileTransferHandler.h"
#include <muduo/base/Logging.h>
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
void FileTransferHandler(const muduo::net::TcpConnectionPtr& conn,
                         const MyProtoMsg& msg,
                         void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server-> redis();
    std::string username = redis.get("token:"+ msg.body["token"].asString());
    if(username.empty())
    {
        return;   // token 过期,不转发
    }
    const_cast<MyProtoMsg&>(msg).body["from"] = username;   // 加上"谁发的"
    if(msg.head.server == 102)
    {
        Json::Value meta;
        meta["type"] = "file";
        meta["from"] = username;
        meta["to"] = msg.body["to"].asString();
        meta["filename"] = msg.body["filename"].asString();
        meta["filesize"] = msg.body["filesize"].asInt();
        server->mysql().SaveHistory(meta, username,msg.body["to"].asString(),msg.body["group"].asString());
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(const_cast<MyProtoMsg*>(&msg),len);
    std::string to = msg.body["to"].asString();
    std::string group = msg.body["group"].asString();
    if(!to.empty())
    {
        auto Friendconn = server->GetconnByUser(to);
        if(Friendconn)
        {
            Friendconn -> send(data,len);
        }
    }
    else if(!group.empty())
    {
        std::string groupid = server->NameToId(group);
        typedef std::vector<std::string> ves;
        ves members = redis.smembers("group:" + groupid + ":members");
        for(size_t i = 0;i < members.size(); i++)
        {
            if(members[i] == username) continue;
            auto memberconn = server->GetconnByUser(members[i]);
            if(memberconn)
            {
                memberconn -> send(data,len);
            }
        }
    }
    delete[] data;
}
   