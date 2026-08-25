#pragma once
#include <stdio.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
#include "../server/Router.h"
#include "../protocal/pack.h"
#include "../protocal/unpack.h"
#include <json/json.h>
#include <muduo/net/TimerId.h>
#include <atomic>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
class MyProtoMsg;
class ChatClient
{
public:
    ChatClient(muduo::net::EventLoop* loop,const muduo::net::InetAddress& ServerAddr);
    void send(uint16_t type, const Json::Value& body);
    void setToken(const std::string& t) {_token = t;}
    std::string getToken() const { return _token; }
    // 登录结果(事件循环线程写,菜单线程读)
    void setLoginResult(bool ok) { _loginOk.store(ok); _loginDone.store(true); }
    bool loginDone() const { return _loginDone.load(); }
    bool loginOk() const { return _loginOk.load(); }
    void resetLoginResult() { _loginDone.store(false); _loginOk.store(false); }
    // 好友/群列表缓存(HandleList 填,菜单线程读)
    void setFriendsCache(const std::vector<std::string>& v) { _friendsCache = v; _friendsDone.store(true); }
    const std::vector<std::string>& friendsCache() const { return _friendsCache; }
    bool friendsDone() const { return _friendsDone.load(); }
    void resetFriends() { _friendsDone.store(false); _friendsCache.clear(); }

    void setGroupsCache(const std::vector<std::string>& v) { _groupsCache = v; _groupsDone.store(true); }
    const std::vector<std::string>& groupsCache() const { return _groupsCache; }
    bool groupsDone() const { return _groupsDone.load(); }
    void resetGroups() { _groupsDone.store(false); _groupsCache.clear(); }
    //
    void setFilesCache(const std::vector<std::string>& v) { _filesCache = v; _filesDone.store(true); }
    const std::vector<std::string>& filesCache() const { return _filesCache; }
    bool filesDone() const { return _filesDone.load(); }
    void resetFiles() { _filesDone.store(false); _filesCache.clear(); }
    //
    FILE* getRecvFile() const { return _recvFile; }
    void setRecvFile(FILE* f) { _recvFile = f; }
    //
    size_t getRecvBytes() const { return _recvBytes; }
    void setRecvBytes(size_t n) { _recvBytes = n; }
    void setRecvFileName(const std::string& f) { _recvFileName = f; }
    std::string getRecvFileName() const { return _recvFileName; }
    
    void setRecvFileId(const std::string& id) { _recvFileId = id; }
    std::string getRecvFileId() const { return _recvFileId; }
    void setRecvFileSize(size_t n) { _recvFileSize = n; }
    size_t getRecvFileSize() const { return _recvFileSize; }
    void setRecvLastPercent(int p) { _recvLastPercent = p; }
    int getRecvLastPercent() const { return _recvLastPercent; }

    std::string getPendingPath() const { return _pendingPath; }
    void setPendingPath(const std::string& p) { _pendingPath = p; }
    std::string getPendingTo() const { return _pendingTo; }
    void setPendingTo(const std::string& t) { _pendingTo = t; }
    std::string getPendingGroup() const { return _pendingGroup; }
    void setPendingGroup(const std::string& g) { _pendingGroup = g; }
    
    void print(const std::string& s);   // 把要显示的文本丢进打印队列
    void start();
    void forceQuit() 
    { 
        _loop->quit(); 
    }
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
    //
    std::atomic<bool> _loginDone{false};
    std::atomic<bool> _loginOk{false};
    std::vector<std::string> _friendsCache;
    std::atomic<bool> _friendsDone{false};
    std::vector<std::string> _groupsCache;
    std::atomic<bool> _groupsDone{false};
    //
    FILE* _recvFile = nullptr;
    //
    size_t _recvBytes = 0;         
    std::string _recvFileName;
    
    std::string _recvFileId;
    size_t _recvFileSize = 0;
    int _recvLastPercent = -1;

    std::string _pendingPath;  
    std::string _pendingTo;      
    std::string _pendingGroup;  

    std::queue<std::string> _printQueue;
    std::mutex _printMutex;
    std::condition_variable _printCond;
    void printLoop();

    //
    std::vector<std::string> _filesCache;
    std::atomic<bool> _filesDone{false};
    //

};
void ySendFile(ChatClient* client, std::string yPath, std::string to_somebody,std::string groupname, int resumeOffset = 0);



