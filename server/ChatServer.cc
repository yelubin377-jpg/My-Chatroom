#include "ChatServer.h"
#include <muduo/base/Logging.h>
#include <boost/bind/bind.hpp>  //只能给C++用
#include "../protocal/protocal.h"
#include "../protocal/pack.h"
#include "../protocal/unpack.h"
#include "../BusinessLogic/AddFriendHandler.h"
#include "../BusinessLogic/AddGroupAdminHandler.h"
#include "../BusinessLogic/ApproveJoinHandler.h"
#include "../BusinessLogic/BlockFriendHandler.h"
#include "../BusinessLogic/CreateGroupHandler.h"
#include "../BusinessLogic/DeleteFriendHandler.h"
#include "../BusinessLogic/DeleteGroupHandler.h"
#include "../BusinessLogic/GroupChatHandler.h"
#include "../BusinessLogic/JoinGroupHandler.h"
#include "../BusinessLogic/LeaveGroupHandler.h"
#include "../BusinessLogic/LoginHandler.h"
#include "../BusinessLogic/LogoutHandler.h"
#include "../BusinessLogic/DeleteAccountHandler.h"
#include "../BusinessLogic/MessageType.h"
#include "../BusinessLogic/PrivateChatHandler.h"
#include "../BusinessLogic/QueryFriendHandler.h"
#include "../BusinessLogic/QueryGroupMembersHandler.h"
#include "../BusinessLogic/QueryUserGroupsHandler.h"
#include "../BusinessLogic/RegisterHandler.h"
#include "../BusinessLogic/RemoveGroupAdminHandler.h"
#include "../BusinessLogic/RemoveGroupMembersHandler.h"
#include "../BusinessLogic/UnBlockFriendHandler.h"
#include "../BusinessLogic/QueryHistoryHandler.h"
#include "../BusinessLogic/FileTransferHandler.h"
#include "../BusinessLogic/EmailSender.h"
#include "../BusinessLogic/GetVerifyCodeHandler.h"
#include "../BusinessLogic/AcceptFriendHandler.h"
#include "../BusinessLogic/RejectFriendHandler.h"
#include "../BusinessLogic/ResetPasswordHandler.h"
#include "../BusinessLogic/LoginByCodeHandler.h"

ChatServer::ChatServer(muduo::net::EventLoop* loop,
                       const muduo::net::InetAddress& listenAddr)
    :_loop(loop),
    _server(loop,listenAddr,"ChatServer"),
    _redis("127.0.0.1",6379)
    ,_mysql("127.0.0.1","chat","2643534502","chatroom",3306)
    ,_email("2643534502@qq.com","uivwjgtnfouweajb")
{
    _redis.connect();
    _mysql.connect();
    _server.setConnectionCallback(
        std::bind(&ChatServer::onConnection, this,
                  std::placeholders::_1));
    _server.setMessageCallback(
        std::bind(&ChatServer::onMessage, this,
                  std::placeholders::_1,
                  std::placeholders::_2,
                  std::placeholders::_3));
    registerHandlers();
}
//登录后拿用户名 对应 相应的与连接有关的指针
void ChatServer::AddOnlineUser(const std::string& username,
                               const muduo::net::TcpConnectionPtr& conn)
{
     muduo::net::TcpConnectionPtr oldConn;
    {
        std::lock_guard<std::mutex> lock(_onlineMutex);
        
        // 顶号：同用户名已在线 → 记下旧连接，清映射
        auto it = _UserToconn.find(username);
        if(it != _UserToconn.end() && it->second != conn)
        {
            oldConn = it->second;
            _connToUser.erase(oldConn);
            _UserToconn.erase(it);
        }
        // 幽灵：同连接换账号 → 直接操作 _connToUser(不调 usernameByconn, 避免死锁)
        auto it2 = _connToUser.find(conn);
        if(it2 != _connToUser.end() && it2->second != username)
        {
            _UserToconn.erase(it2->second);
        }
        // 登记新账号
        _UserToconn[username] = conn;
        _connToUser[conn] = username;
    }

    // 锁外发顶号通知(网络操作, 不能锁内做)
    if(oldConn)
    {
        MyProtoMsg kick;
        kick.head.server = 912;
        kick.body["from"] = "system";
        kick.body["msg"] = "你已在其他设备登录, 请重新登录";
        MyProtoEncode encoder;
        uint32_t len = 0;
        uint8_t* data = encoder.encode(&kick, len);
        oldConn->send(data, len);
        delete[] data;
    }
}
bool ChatServer::TrueOnline(const std::string& username) const
{
    std::lock_guard<std::mutex> lock(_onlineMutex);
    if(_UserToconn.find(username) != _UserToconn.end()) //不是最后一个，证明找到了，true
    {
        return true;
    }
    else return false;
}
//直接关闭窗口后拿与连接有关的指针 对应 相应的用户名
std::string ChatServer::usernameByconn(const muduo::net::TcpConnectionPtr& conn) const
{
    std::lock_guard<std::mutex> lock(_onlineMutex);
    auto it = _connToUser.find(conn);
    if(it != _connToUser.end())
    {
        return it->second;
    }
    else {return "";}
}
void ChatServer::RemoveOnlineUser(const std::string& username)
{
    std::lock_guard<std::mutex> lock(_onlineMutex);
    auto it = _UserToconn.find(username);
    if( it != _UserToconn.end())
    {
        auto conn = it->second;//
        _UserToconn.erase(it);
        _connToUser.erase(it->second);
    }
}
muduo::net::TcpConnectionPtr ChatServer::GetconnByUser(const std::string& username) const
{
    std::lock_guard<std::mutex> lock(_onlineMutex);
    auto it = _UserToconn.find(username);
    if(it != _UserToconn.end())
    {
        return it->second;
    }
    return muduo::net::TcpConnectionPtr();
}
void ChatServer::start()
{
    _heartbeatTimerId  = _loop->runEvery(HeartBeatInterval,std::bind(&ChatServer::onHeartbeat,this));
    //
    _server.setThreadNum(4);
    //
    _server.start();
}
void ChatServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    LOG_INFO << "ChatServer(onConnection) - " << conn->peerAddress().toIpPort()
             << " -> " << conn-> localAddress().toIpPort() 
             << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if(conn->connected())
    {
        _conns.insert(conn);
        _LastPong[conn] = muduo::Timestamp::now();
    }
    else
    {
        {
            std::lock_guard<std::mutex> lock(_onlineMutex);
            auto it = _connToUser.find(conn);
            if(it != _connToUser.end())
            {
                _UserToconn.erase(it->second);
                _connToUser.erase(it);
            }
        }
        _conns.erase(conn);
        _LastPong.erase(conn);
    }
}
void ChatServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
                           muduo::net::Buffer* buf,
                           muduo::Timestamp time)
{
    while(1)
    {
    MyProtoMsg *outMsg = nullptr;
    int result = _decode.decode(buf,outMsg);
if(result == 0) return;
    if(result == -1) {conn->shutdown();return;}

    uint16_t type = outMsg->head.server;
if(type == static_cast<uint16_t>(MessageType::HEARTBEAT))
{
    _LastPong[conn] = muduo::Timestamp::now();
    delete outMsg;
    continue;
}
    _router.dispatch(type,conn,*outMsg,this);
    delete outMsg;}
}


void ChatServer::registerHandlers()
{
    _router.on(66,HandleLogin);
    _router.on(666,HandleRegister);
    _router.on(68,LogoutHandler);
    _router.on(69,DeleteAccountHandler);
    _router.on(86,AddFriendHandler);
    _router.on(2025,DeleteFriendHandler);
    _router.on(2026,QueryFriendHandler);
    _router.on(2027,BlockFriendHandler);
    _router.on(2028,UnBlockFriendHandler);
    _router.on(729,PrivateChatHandler);
    _router.on(1950,CreateGroupHandler);
    _router.on(2007,JoinGroupHandler);  
    _router.on(06,GroupChatHandler);
    _router.on(12,AddGroupAdminHandler);
    _router.on(13,ApproveJoinHandler);
    _router.on(14,DeleteGroupHandler);
    _router.on(15,LeaveGroupHandler);
    _router.on(16,QueryGroupMembersHandler);
    _router.on(17,QueryUserGroupsHandler);
    _router.on(19,RemoveGroupAdminHandler);
    _router.on(18,RemoveGroupMembersHandler);
    _router.on(811,QueryHistoryHandler);
    _router.on(100,FileTransferHandler);//靠 head.server
    _router.on(101,FileTransferHandler);
    _router.on(102,FileTransferHandler);
    _router.on(103,FileTransferHandler);
    _router.on(104,FileTransferHandler);
    _router.on(105,GetVerifyCodeHandler);
    _router.on(106,ResetPasswordHandler);
    _router.on(107,AcceptFriendHandler);
    _router.on(108,RejectFriendHandler);
    _router.on(109,LoginByCodeHandler);
    _router.on(110,ListFilesHandler);
    _router.on(111,DownloadFileHandler);
    _router.on(112,ListFriendRequestsHandler);
}

void ChatServer::onHeartbeat()
{
    muduo::Timestamp now = muduo::Timestamp::now();
    std::vector<muduo::net::TcpConnectionPtr> deadConns;

    for(auto it = _LastPong.begin();it != _LastPong.end();)
    {
        double elapsed = now.secondsSinceEpoch() - it->second.secondsSinceEpoch();
        if(elapsed > HeartBeatTimeout)
        {
            LOG_INFO << "Heartbeat timeout:" << it->first->peerAddress().toIpPort();
            deadConns.push_back(it->first);

            {
                std::lock_guard<std::mutex> lock(_onlineMutex);
                _UserToconn.erase(_connToUser[it->first]);
                _connToUser.erase(it->first);
            }
            _conns.erase(it->first);
            it = _LastPong.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for(auto& c : deadConns)
    {
        c->shutdown();
    }
}

std::string ChatServer::NameToId(const std::string& groupname)
{
    return _redis.get("groupname:" + groupname);
}