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
        std::cout << "[ok] 登陆成功" << std::endl;
    }
    else
    {
        std::cout << "[Fail]" << msg.body["msg"].asString() << std::endl;
    }
}

void HandleDefault(const muduo::net::TcpConnectionPtr& conn,
                   const MyProtoMsg& msg,
                   void* ctx)
{
    std::string from = msg.body["from"].asString();
    std::string text = msg.body["msg"].asString();
    if(!text.empty())
    {
        if(!from.empty())
        {
            std::cout << "[" << from << "]" << text << std::endl;
        }
        else
        {
            std::cout << text << std::endl;
        }
    }
    if(msg.body.isMember("ai_reply"))
    {
        std::cout << "[Artificial intelligence]" << msg.body["ai_reply"].asString() << std::endl;
    }
}
void HandleList(const muduo::net::TcpConnectionPtr& conn,
                const MyProtoMsg& msg, void* ctx)
{
    std::cout << msg.body["msg"].asString() << std::endl;
    if(msg.body["status"].asString() != "ok") return;
    if(msg.body.isMember("friends"))
    {
        const Json::Value& arr = msg.body["friends"];
        for(int i = 0; i < arr.size(); i++)
            std::cout << arr[i].asString() << std::endl;
    }
    if(msg.body.isMember("members"))
    {
        const Json::Value& arr = msg.body["members"];
        for(int i = 0; i < arr.size(); i++)
            std::cout << arr[i].asString() << std::endl;
    }
    if(msg.body.isMember("groups"))
    {
        const Json::Value& arr = msg.body["groups"];
        for(int i = 0; i < arr.size(); i++)
        {
            std::cout << arr[i].asString() << std::endl;
    
        }
    }
}
//
void HandleHistory(const muduo::net::TcpConnectionPtr& conn,
                   const MyProtoMsg& msg,
                   void* ctx)
{
    std::cout << msg.body["msg"].asString() << std::endl;
    if(msg.body["status"].asString() != "ok") return;
    const Json::Value& history = msg.body["history"];
    for(int i = 0; i < history.size(); i++)
    {
        std::string who = history[i]["User"].asString();
        std::string text = history[i]["msg"].asString();
        std::string time = history[i]["Time"].asString();
        std::cout << "[" << who << " " << time << "] " << text << std::endl;
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
    int resumeOffset = msg.body.get("resumeOffset", 0).asInt();
    std::cout << "[file] " << from << " 发来文件: " << filename << (resumeOffset > 0 ? " (续传)" : "") << std::endl;
    client->setRecvFileName(filename);
    client->setRecvBytes(resumeOffset);
    client->setRecvFile(fopen((filename + ".part").c_str(), "ab"));
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
        std::string filename = client->getRecvFileName();
        rename((filename + ".part").c_str(), filename.c_str());
        remove((filename + ".offset").c_str());
        std::cout << "[file] 文件接收完成" << std::endl;
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
    ChatClient* client = static_cast<ChatClient*>(ctx);
    client->setToken("");   // 清 token
    std::cout << "[!] 你已在其他设备登录，已自动退出" << std::endl;
}
void HandleResumeReply(const muduo::net::TcpConnectionPtr& conn,
                       const MyProtoMsg& msg,
                       void* ctx)
{
    ChatClient* client = static_cast<ChatClient*>(ctx);
    int offset = msg.body["offset"].asInt();
    std::cout << "[file] 对方已有 " << offset << " 字节,继续发送" << std::endl;
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
    _router.on(109, HandleDefault);
    _router.on(106,HandleDefault);
    _router.on(107,HandleDefault);
    _router.on(108,HandleDefault);
    
}       


void ChatClient::onHeartbeat()
{
    Json::Value body;
    send(static_cast<uint16_t>(MessageType::HEARTBEAT),body);
}
