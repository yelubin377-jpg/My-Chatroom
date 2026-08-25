#include "Client.h"
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include <boost/bind/bind.hpp>
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../protocal/unpack.h"
#include "../BusinessLogic/MessageType.h"
#include "base64.h"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <ctime>

ChatClient::ChatClient(muduo::net::EventLoop* loop,
                       const muduo::net::InetAddress& ServerAddr)
        :_loop(loop),
        _client(loop,ServerAddr,"ChatClient")
{
    _client.setConnectionCallback(
        boost::bind(&ChatClient::onConnection,this,
                    std::placeholders::_1)
    );
    _client.setMessageCallback(
        boost::bind(&ChatClient::onMessage,this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3)
    );
    registerHandlers();
    std::thread t(&ChatClient::printLoop, this);
    t.detach();
}
void ChatClient::start()
{
    _client.connect();
}
void ChatClient::send(uint16_t type, const Json::Value& body)
{
    MyProtoMsg msg;
    msg.head.server = type;
    msg.body = body;
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&msg,len);
    if(_conn) _conn->send(data,len);
    delete[] data;
}
//长文本接受
void ChatClient::print(const std::string& s)
{
    std::lock_guard<std::mutex> lock(_printMutex);
    _printQueue.push(s);
    _printCond.notify_one();
}
void ChatClient::printLoop()
{
    while(true)
    {
        std::unique_lock<std::mutex> lock(_printMutex);
        _printCond.wait(lock, [this]{ return !_printQueue.empty(); });
        std::string s = _printQueue.front();
        _printQueue.pop();
        lock.unlock();
        std::cout << s << std::flush;
    }
}
//
void ChatClient::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    LOG_INFO << "onConnection - (ChatClient)" << conn->peerAddress().toIpPort()
             << " -> " << conn->localAddress().toIpPort()  
             << " is " << (conn->connected() ? "UP" : "DOWN");
    if(conn->connected())
    {
        _conn = conn;
        _heartbeatTimerId = _loop->runEvery(HeartBeatInterval,std::bind(&ChatClient::onHeartbeat,this));
    }
    else
    {
        _loop->cancel(_heartbeatTimerId);
        _conn.reset();

    }
}
void ChatClient::onMessage(const muduo::net::TcpConnectionPtr& conn,
                           muduo::net::Buffer* buf,
                           muduo::Timestamp time)
{
while(true)
  {
    MyProtoMsg* outMsg = nullptr;
    int result = _decode.decode(buf,outMsg);
    if(result == 0) return;
    if(result ==-1) 
    {
        conn->shutdown();
        return;
    }
    uint16_t type = outMsg->head.server;
    _router.dispatch(type,conn,*outMsg,this);
    delete outMsg;
  }
}


void HandleLoginResponse(const muduo::net::TcpConnectionPtr& conn,
                         const MyProtoMsg& msg,
                         void* ctx)
{
    ChatClient* client = static_cast<ChatClient*> (ctx);
    std::string status = msg.body["status"].asString();
    if(status == "ok")
    {
        client -> setToken(msg.body["token"].asString());
        client -> setLoginResult(true);
        client->print("[ok] 登陆成功\n");
    }
    else
    {
        client -> setLoginResult(false);
        client->print("[Fail]" + msg.body["msg"].asString() + "\n");
    }
}

void HandleDefault(const muduo::net::TcpConnectionPtr& conn,
                   const MyProtoMsg& msg,
                   void* ctx)
{
    ChatClient* client = static_cast<ChatClient*>(ctx);
    std::string from = msg.body["from"].asString();
    std::string text = msg.body["msg"].asString();
    if(!text.empty())
    {
        if(!from.empty())
        {
            client->print("[" + from + "]" + text + "\n");
        }
        else
        {
            client->print(text + "\n");
        }
    }
}
void HandleList(const muduo::net::TcpConnectionPtr& conn,
                const MyProtoMsg& msg, void* ctx)
{
    ChatClient* client = static_cast<ChatClient*>(ctx);
    client->print(msg.body["msg"].asString() + "\n");
    if(msg.body["status"].asString() != "ok") return;
    if(msg.body.isMember("friends"))
    {
        std::vector<std::string> v;
        const Json::Value& arr = msg.body["friends"];
        for(int i = 0; i < arr.size(); i++)
            v.push_back(arr[i].asString());
        client->setFriendsCache(v);
    }
    if(msg.body.isMember("members"))
    {
        const Json::Value& arr = msg.body["members"];
        for(int i = 0; i < arr.size(); i++)
        client->print(arr[i].asString() + "\n");
    }
    if(msg.body.isMember("groups"))
    {
        std::vector<std::string> v;
        const Json::Value& arr = msg.body["groups"];
        for(int i = 0; i < arr.size(); i++)
            v.push_back(arr[i].asString());
        client->setGroupsCache(v);
    }
}
//
void HandleFileList(const muduo::net::TcpConnectionPtr& conn,
                    const MyProtoMsg& msg,
                    void* ctx)
{
    ChatClient* client = static_cast<ChatClient*>(ctx);
    std::vector<std::string> v;
    if(msg.body.isMember("files"))
    {
        const Json::Value& arr = msg.body["files"];
        for(int i = 0; i < arr.size(); i++)
        {
            v.push_back(arr[i].asString());
        }
    }
    client->setFilesCache(v);
}
//
void HandleHistory(const muduo::net::TcpConnectionPtr& conn,
                   const MyProtoMsg& msg,
                   void* ctx)
{
    ChatClient* client = static_cast<ChatClient*>(ctx);
    client->print(msg.body["msg"].asString() + "\n");
    if(msg.body["status"].asString() != "ok") return;
    const Json::Value& history = msg.body["history"];
    for(int i = 0; i < history.size(); i++)
    {
        std::string who = history[i]["User"].asString();
        std::string text = history[i]["msg"].asString();
        std::string time = history[i]["Time"].asString();
        client->print("[" + who + " " + time + "] " + text + "\n");
    }
}
//
void HandleFileStart(const muduo::net::TcpConnectionPtr& conn,
                     const MyProtoMsg& msg,
                     void* ctx)
{
    ChatClient* client = static_cast<ChatClient*>(ctx);
    std::string from = msg.body["from"].asString();
    std::string filename = msg.body["filename"].asString();
    std::string fileId = msg.body["fileId"].asString();
    int resumeOffset = msg.body.get("resumeOffset", 0).asInt();
    long long filesize = msg.body.get("filesize", 0).asInt64();
    client->setRecvFileSize((size_t)filesize);
    client->setRecvLastPercent(-1);
    client->print("[file] " + from + " 发来文件: " + filename + (resumeOffset > 0 ? " (续传)" : "") + "\n");
    client->setRecvFileName(filename);
    client->setRecvFileId(fileId);
    client->setRecvBytes(resumeOffset);
    const char* mode = (resumeOffset > 0) ? "ab" : "wb";
    client->setRecvFile(fopen((fileId + ".part").c_str(), mode));
}
void HandleFileData(const muduo::net::TcpConnectionPtr& conn,
                    const MyProtoMsg& msg,
                    void* ctx)
{
    ChatClient* client = static_cast<ChatClient*>(ctx);
    if(!client->getRecvFile()) return;
    std::string raw = base64::decode(msg.body["data"].asString());
    fwrite(raw.c_str(), 1, raw.size(), client->getRecvFile());
    client->setRecvBytes(client->getRecvBytes()+raw.size());
    size_t total = client->getRecvFileSize();
    if(total > 0)
    {
        int percent = (int)(client->getRecvBytes() * 100 / total);
        if(percent != client->getRecvLastPercent())
        {
            client->setRecvLastPercent(percent);
            client->print("[接收] " + client->getRecvFileName() + " " + std::to_string(percent) + "%\n");
        }
    }
    static int count = 0;
    count++;
    if(count % 50 == 0)
    {
        FILE* of = fopen((client->getRecvFileName() + ".offset").c_str(), "w");
        if(of)
        {
            fprintf(of, "%zu", client->getRecvBytes());
            fclose(of);
        }
    }
}
void HandleFileEnd(const muduo::net::TcpConnectionPtr& conn,
                   const MyProtoMsg& msg,
                   void* ctx)
{
    ChatClient* client = static_cast<ChatClient*>(ctx);
    if(client->getRecvFile())
    {
        fclose(client->getRecvFile());
        client->setRecvFile(nullptr);
        std::string fileId = client->getRecvFileId();
        std::string filename = client->getRecvFileName();
        std::string finalName = filename;
        FILE* check = fopen(finalName.c_str(), "r");
        if(check)
        {
            fclose(check);
            finalName = filename + "_" + std::to_string(time(nullptr));
        }
        rename((fileId + ".part").c_str(), finalName.c_str());
        remove((filename + ".offset").c_str());
        client->print("[file] 文件接收完成: " + finalName + "\n");
    }
}
void HandleResumeQuery(const muduo::net::TcpConnectionPtr& conn,
                       const MyProtoMsg& msg,
                       void* ctx)
{
    ChatClient* client = static_cast<ChatClient*>(ctx);
    std::string filename = msg.body["filename"].asString();
    Json::Value reply;
    reply["token"] = client->getToken();          // 关键token ， 挂了号多次就是这里没加
    reply["to"] = msg.body["from"].asString();     
    reply["filename"] = filename;
    reply["offset"] = 0;
    FILE* of = fopen((filename + ".offset").c_str(), "r");
    if(of)
    {
        int n = 0;
        if(fscanf(of, "%d", &n) == 1) reply["offset"] = n;
        fclose(of);
    }
    client->send(static_cast<uint16_t>(MessageType::FILE_RESUME_REPLY), reply);
}
void HandleSecondLogin(const muduo::net::TcpConnectionPtr& conn,
                       const MyProtoMsg& msg,
                       void* ctx)
{
    std::cerr << "\n[!] 你已在其他设备登录，本客户端已退出\n";
    _Exit(0);
}
void HandleResumeReply(const muduo::net::TcpConnectionPtr& conn,
                       const MyProtoMsg& msg,
                       void* ctx)
{
    ChatClient* client = static_cast<ChatClient*>(ctx);
    int offset = msg.body["offset"].asInt();
    client->print("[file] 对方已有 " + std::to_string(offset) + " 字节,继续发送\n");
    std::string pPath = client->getPendingPath();
    std::string pTo = client->getPendingTo();
    std::string pGroup = client->getPendingGroup();
    std::thread t([client, pPath, pTo, pGroup, offset]() 
    {
        ySendFile(client, pPath, pTo, pGroup, offset);
    });
    t.detach();
}

//
void ChatClient::registerHandlers()
{

    _router.on(66,HandleLoginResponse);
    _router.on(666,HandleDefault);
    _router.on(68,HandleDefault);
    _router.on(69,HandleDefault);
    _router.on(86,HandleDefault);
    _router.on(2025,HandleDefault);
    _router.on(2026,HandleList);
    _router.on(2027,HandleDefault);
    _router.on(2028,HandleDefault);
    _router.on(729,HandleDefault);
    _router.on(1950,HandleDefault);
    _router.on(2007,HandleDefault);  
    _router.on(06,HandleDefault);
    _router.on(12,HandleDefault);
    _router.on(13,HandleDefault);
    _router.on(14,HandleDefault);
    _router.on(15,HandleDefault);
    _router.on(16,HandleList);
    _router.on(17,HandleList);
    _router.on(19,HandleDefault);
    _router.on(18,HandleDefault);
    _router.on(811,HandleHistory);
    _router.on(911,HandleDefault);
    _router.on(100,HandleFileStart);
    _router.on(101,HandleFileData);
    _router.on(102,HandleFileEnd);
    _router.on(103,HandleResumeQuery);
    _router.on(104,HandleResumeReply);
    _router.on(105, HandleDefault);
    _router.on(912,HandleSecondLogin); 
    _router.on(109, HandleLoginResponse);
    _router.on(106,HandleDefault);
    _router.on(107,HandleDefault);
    _router.on(108,HandleDefault);
    _router.on(110,HandleFileList);
    _router.on(112,HandleList);
}       


void ChatClient::onHeartbeat()
{
    Json::Value body;
    send(static_cast<uint16_t>(MessageType::HEARTBEAT),body);
}
