#pragma once
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <set>
#include <map>
#include "Router.h"
#include "../protocal/unpack.h"
#include "../BusinessLogic/redisClient.h"
#include <muduo/net/TimerId.h>   
#include "../BusinessLogic/AIclient.h"
#include "../BusinessLogic/MySqlClient.h"
#include "../BusinessLogic/EmailSender.h"
#include <mutex>
class MyProtoMsg;
class ChatServer
{
public:
    ChatServer(muduo::net::EventLoop* loop,
               const muduo::net::InetAddress& listenAddr);
    redisClient& redis() {return _redis;}
    AIclient& ai() {return _ai;}
    MySqlClient& mysql() {return _mysql;}              
    void AddOnlineUser(const std::string& username,
                       const muduo::net::TcpConnectionPtr& conn);
    bool TrueOnline(const std::string& username) const; 
    void RemoveOnlineUser(const std::string& username);
    muduo::net::TcpConnectionPtr GetconnByUser(const std::string& username) const;
    std::string NameToId(const std::string& groupname);
    void start();
    EmailSender& email() {return _email;}   
private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn);
    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp time);
    void registerHandlers();
    void onHeartbeat();
    muduo::net::TimerId _heartbeatTimerId;
    std::map<muduo::net::TcpConnectionPtr,muduo::Timestamp> _LastPong;
    muduo::net::EventLoop* _loop;
    muduo::net::TcpServer _server;
    Router _router;
    MyProtoDecode _decode;
    std::set<muduo::net::TcpConnectionPtr> _conns;
    redisClient _redis;
    AIclient _ai;
    MySqlClient _mysql;
    EmailSender _email;
    std::map<std::string , muduo::net::TcpConnectionPtr> _UserToconn;
    std::map<muduo::net::TcpConnectionPtr , std::string> _connToUser;
    std::string usernameByconn(const muduo::net::TcpConnectionPtr& conn) const;
    static constexpr double HeartBeatInterval = 10.0;
    static constexpr double HeartBeatTimeout = 15.0;
    mutable std::mutex _onlineMutex;
    
    
};
