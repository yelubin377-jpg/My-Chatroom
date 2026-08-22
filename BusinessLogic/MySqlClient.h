#pragma once
#include <mysql/mysql.h>
#include <json/json.h>
#include <string>
#include <vector>
#include <mutex>
class MySqlClient
{
public:
    MySqlClient(std::string host , std::string username,std::string password , std::string DataBaseName, 
        int port);
    ~MySqlClient();
    // bool SaveHistory(Json::Value body  , std::string host,std::string username, std::string password , std::string friendname
    //  , int port , MYSQL* conn);
    bool SaveHistory(Json::Value body,std::string username,std::string friendname , std::string groupid);
    bool SaveOffline(Json::Value body,std::string username ,std::string friendname  ,std::string groupid);
 
                   //GetPrivateHistory
    std::vector<Json::Value> GPH(std::string username,  std::string FriendName,  int limit);
    
     //Get GroupHistory
    std::vector<Json::Value> GGH(std::string username, std::string groupid,int limit);
    //复盘点：再上面两个函数的参数是否有Json::value 卡了一会儿，还有返回值类型，后面回来复盘的时候可以重点关注一下
    std::vector<Json::Value> PushOffline(std::string username);
    bool ClearOffline(std::string username);
    bool connect();
    

private:
    MYSQL* _conn;
    std::string _host;
    std::string _username;
    std::string _password;
    std::string _DataBaseName;
    std::mutex _mutex;
    int _port;

};