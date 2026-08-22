#pragma once
#include <stdio.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
#include "../server/Router.h"
#include "../protocal/pack.h"
#include "../protocal/unpack.h"
#include <json/json.h>
#include <muduo/net/TimerId.h>
class MyProtoMsg;
class ChatClient
{
public:
    ChatClient(muduo::net::EventLoop* loop,const muduo::net::InetAddress& ServerAddr);
    void send(uint16_t type, const Json::Value& body);
    void setToken(const std::string& t) {_token = t;}
    std::string getToken() const { return _token; }
    FILE* getRecvFile() const { return _recvFile; }
    void setRecvFile(FILE* f) { _recvFile = f; }
    //
    size_t getRecvBytes() const { return _recvBytes; }
    void setRecvBytes(size_t n) { _recvBytes = n; }
    void setRecvFileName(const std::string& f) { _recvFileName = f; }
    std::string getRecvFileName() const { return _recvFileName; }

    std::string getPendingPath() const { return _pendingPath; }
    void setPendingPath(const std::string& p) { _pendingPath = p; }
    std::string getPendingTo() const { return _pendingTo; }
    void setPendingTo(const std::string& t) { _pendingTo = t; }
    std::string getPendingGroup() const { return _pendingGroup; }
    void setPendingGroup(const std::string& g) { _pendingGroup = g; }
    
    void start();
private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn);
    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp time);
    void registerHandlers();
    muduo::net::TcpClient _client;
    muduo::net::EventLoop* _loop;
    Router _router;
    MyProtoEncode _encode;
    MyProtoDecode _decode;
    muduo::net::TcpConnectionPtr _conn;
    muduo::net::TimerId _heartbeatTimerId;
    void onHeartbeat();
    static constexpr double HeartBeatInterval = 10.0;
    std::string _token;
    FILE* _recvFile = nullptr;
    //
    size_t _recvBytes = 0;         
    std::string _recvFileName;
    
    std::string _pendingPath;  
    std::string _pendingTo;      
    std::string _pendingGroup;  
    
};
void ySendFile(ChatClient* client, std::string yPath, std::string to_somebody,std::string groupname, int resumeOffset = 0);



