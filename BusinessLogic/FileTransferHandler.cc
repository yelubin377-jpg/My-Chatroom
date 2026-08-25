#include "FileTransferHandler.h"
#include <muduo/base/Logging.h>
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../server/ChatServer.h"
#include "redisClient.h"
#include <map>
#include <mutex>
#include <sys/stat.h>
#include "../Client/base64.h"
static std::map<std::string, FILE*> g_serverFiles;
static std::mutex g_fileMutex;
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
    std::string fileId = msg.body["fileId"].asString();
    std::string to = msg.body["to"].asString();
    std::string group = msg.body["group"].asString();
    if(msg.head.server == 100)   // 文件开始
    {
        mkdir("server_files", 0755);
        int resumeOffset = msg.body.get("resumeOffset", 0).asInt();
        FILE* f = fopen(("./server_files/" + fileId).c_str(), resumeOffset > 0 ? "ab" : "wb");
        std::lock_guard<std::mutex> lock(g_fileMutex);
        g_serverFiles[fileId] = f;
    }
    else if(msg.head.server == 101)   // 数据块
    {
        std::string raw = base64::decode(msg.body["data"].asString());
        std::lock_guard<std::mutex> lock(g_fileMutex);
        auto it = g_serverFiles.find(fileId);
        if(it != g_serverFiles.end() && it->second)
            fwrite(raw.c_str(), 1, raw.size(), it->second);
    }
    else if(msg.head.server == 102)   // 文件结束
    {
        std::lock_guard<std::mutex> lock(g_fileMutex);
        auto it = g_serverFiles.find(fileId);
        if(it != g_serverFiles.end())
        {
            if(it->second) fclose(it->second);
            g_serverFiles.erase(it);
        }
    }
    if(msg.head.server == 102)
    {
        Json::Value meta;
        meta["type"] = "file";
        meta["from"] = username;
        meta["to"] = msg.body["to"].asString();
        meta["filename"] = msg.body["filename"].asString();
        meta["filesize"] = msg.body["filesize"].asInt64();
        meta["fileId"] = fileId;
        meta["msg"] = "[文件] "+msg.body["filename"].asString();
        server->mysql().SaveHistory(meta, username,msg.body["to"].asString(),msg.body["group"].asString());
        std::string filename = msg.body["filename"].asString();
        std::string entry = fileId + "|" + filename + "|" + username;
        if(!to.empty())
        {
            redis.lpush("file_list:" + to, entry);
        }
        else if(!group.empty())
        {
            std::string groupid = server->NameToId(group);
            std::vector<std::string> members = redis.smembers("group:" + groupid + ":members");
            for(size_t i = 0; i < members.size(); i++)
            redis.lpush("file_list:" + members[i], entry);
        }
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(const_cast<MyProtoMsg*>(&msg),len);
    if(!to.empty())
    {
        auto Friendconn = server->GetconnByUser(to);
        if(Friendconn)
        {
            Friendconn -> send(data,len);
        }
        else if(msg.head.server == 103)
        {
            MyProtoMsg reply;
            reply.head.server = 104;
            reply.body["offset"] = 0;
            reply.body["filename"] = msg.body["filename"].asString();
            MyProtoEncode enc2;
            uint32_t len2 = 0;
            uint8_t* d2 = enc2.encode(&reply, len2);
            conn->send(d2, len2);
            delete[] d2;
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

void ListFilesHandler(const muduo::net::TcpConnectionPtr& conn,
                      const MyProtoMsg& msg,
                      void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();
    MyProtoMsg response;
    response.head.server = msg.head.server;
    std::string username = redis.get("token:" + msg.body["token"].asString());
    if(username.empty())
    {
        response.body["status"] = "error";
        response.body["msg"] = "token过期,请重新登录";
    }
    else
    {
        std::vector<std::string> list = redis.lrange("file_list:" + username, 0, -1);
        for(size_t i = 0; i < list.size(); i++)
            response.body["files"][(int)i] = list[i];
        response.body["status"] = "ok";
        response.body["msg"] = "文件列表:";
    }
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&response, len);
    conn->send(data, len);
    delete[] data;
}
void DownloadFileHandler(const muduo::net::TcpConnectionPtr& conn,
                         const MyProtoMsg& msg,
                         void* ctx)
{
    ChatServer* server = static_cast<ChatServer*>(ctx);
    redisClient& redis = server->redis();
    std::string username = redis.get("token:" + msg.body["token"].asString());
    if(username.empty()) return;
    std::string fileId = msg.body["fileId"].asString();
    std::string filename = msg.body["filename"].asString();
    long offset = msg.body.get("offset", 0).asInt();

    FILE* f = fopen(("server_files/" + fileId).c_str(), "rb");
    if(!f)
    {
        MyProtoMsg response;
        response.head.server = msg.head.server;
        response.body["status"] = "error";
        response.body["msg"] = "文件不存在";
        MyProtoEncode encoder;
        uint32_t len = 0;
        uint8_t* data = encoder.encode(&response, len);
        conn->send(data, len);
        delete[] data;
        return;
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if(offset > fsize) offset = 0;
    fseek(f, offset, SEEK_SET);

    MyProtoMsg start;
    start.head.server = 100;
    start.body["from"] =msg.body.get("from", "server").asString();
    start.body["filename"] = filename;
    start.body["fileId"] = fileId;
    start.body["filesize"] = fsize; 
    start.body["resumeOffset"] = offset;
    MyProtoEncode enc;
    uint32_t len = 0;
    uint8_t* data = enc.encode(&start, len);
    conn->send(data, len);
    delete[] data;

    char buf[65536];
    while(true)
    {
        int n = fread(buf, 1, sizeof(buf), f);
        if(n <= 0) break;
        MyProtoMsg d;
        d.head.server = 101;
        d.body["fileId"] = fileId;
        d.body["data"] = base64::encode(std::string(buf, n));
        uint8_t* dd = enc.encode(&d, len);
        conn->send(dd, len);
        delete[] dd;
    }
    fclose(f);

    MyProtoMsg end;
    end.head.server = 102;
    end.body["fileId"] = fileId;
    uint8_t* ee = enc.encode(&end, len);
    conn->send(ee, len);
    delete[] ee;
}