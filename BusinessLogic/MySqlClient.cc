#include "MySqlClient.h"
#include <json/json.h>
#include <vector>
#include <string>
#include <mysql/mysql.h>
#include <cstdio>
#include <algorithm>
//转义代码
std::string Esc(MYSQL* conn, const std::string& s)
{
    std::string out(s.size()*2+1, '\0');
    size_t n = mysql_real_escape_string(conn, &out[0], s.c_str(), s.size());
    out.resize(n);
    return out;
}
MySqlClient::MySqlClient(std::string host , std::string username,std::string password , std::string DataBaseName,int port)
        :_host(host)
        ,_username(username),
        _password(password),
        _DataBaseName(DataBaseName),
        _port(port)

{}
MySqlClient::~MySqlClient()
{
    _running = false;
    _queueCond.notify_all();
    if(_writerThread.joinable()) _writerThread.join();
    if(nullptr !=_conn){mysql_close(_conn);_conn = nullptr;}
}

bool MySqlClient::connect()
{
    std::lock_guard<std::mutex> lock(_mutex);
    MYSQL* conn = mysql_init(nullptr);
    _conn = conn;
    if( NULL == mysql_real_connect(_conn,"127.0.0.1","chat","2643534502","chatroom",3306,nullptr,0))
    {
        printf("连接失败:%s\n",mysql_error(_conn));
        return false;
    }
    if(mysql_set_character_set(_conn , "utf8mb4") == 0)
    {
        std::string order = "CREATE TABLE IF NOT EXISTS ChatHistory(id INT AUTO_INCREMENT PRIMARY KEY,from_user VARCHAR(128),to_user VARCHAR(128),to_group VARCHAR(128),content MEDIUMTEXT,created_at DATETIME DEFAULT CURRENT_TIMESTAMP)";
        std::string order0="CREATE TABLE IF NOT EXISTS OFFLINE(id INT AUTO_INCREMENT PRIMARY KEY,from_user VARCHAR(128), to_user VARCHAR(128) ,  to_group VARCHAR(128),content MEDIUMTEXT,  created_at DATETIME DEFAULT CURRENT_TIMESTAMP)";
        mysql_query(_conn , order.c_str());
        mysql_query(_conn,order0.c_str());
        _running = true;
        _writerThread = std::thread(&MySqlClient::writerLoop, this);
        return true;
    }
    else return false;

}

bool MySqlClient::SaveHistory(Json::Value body,std::string username,std::string friendname,std::string groupid)
{
    WriteItem item;
    item.body = body;
    item.username = username;
    item.friendname = friendname;
    item.groupid = groupid;
    item.offline = false;
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _writeQueue.push(item);
    }
    _queueCond.notify_one();
    return true;
}
    std::vector<Json::Value> MySqlClient::GPH(std::string username,  std::string FriendName ,int limit)
    {
        std::lock_guard<std::mutex> lock(_mutex);
         if(limit <= 0) 
         {
            limit=300;
         }

        //写成string有点乱 ，先裸者写一下
        //SELECT from_user , content,created_at FROM ChatHistory
        //WHERE (from_user = username AND to_user = FriendName) OR (from_user =FriendName AND to_user = username)
        //ORDER BY created_at DESC LIMIT 300;
        std::string order3 = "SELECT from_user , content , created_at FROM ChatHistory WHERE (from_user = '"+username+"' AND to_user = '"+FriendName+"') OR (from_user = '"+FriendName+"' AND to_user = '"+username+"') ORDER BY id DESC LIMIT " + std::to_string(limit);
        int judge2 = mysql_query(_conn , order3.c_str());
        if(judge2 != 0) return std::vector<Json::Value>();
        MYSQL_RES* result = mysql_store_result(_conn);
         if(NULL == result)
         {
            fprintf(stderr,"%s\n",mysql_error(_conn));
            return std::vector<Json::Value>();
         }
        my_ulonglong Hang = mysql_num_rows(result); //行
         std::vector<Json::Value> JPH;//JsonPrivateHistory

         for(int i = 0;i<Hang;i++)
        {
                Json::Value History;
            MYSQL_ROW NowHang = mysql_fetch_row(result); //这一行的数据都给他取出来
            Json::Reader Read;
           
            Read.parse(NowHang[1],History);
             History["User"] = NowHang[0];
            History["Time"] =NowHang[2];
           JPH.push_back(History);            
        }
        mysql_free_result(result);
        std::reverse(JPH.begin(), JPH.end());
        return JPH;

    }


 std::vector<Json::Value> MySqlClient::GGH(std::string username, std::string groupid ,int limit)
 {
    std::lock_guard<std::mutex> lock(_mutex);
    if(limit <= 0)
    {
        limit =300;
    }
    std::string order5 = "SELECT from_user ,to_group,content, created_at FROM ChatHistory WHERE (to_group = " + groupid + ") ORDER BY id DESC LIMIT "+ std::to_string(limit);
    int judge5 = mysql_query(_conn ,order5.c_str());
    if(judge5 != 0)
    {
        fprintf(stderr,"%s\n",mysql_error(_conn));
         return std::vector<Json::Value>();
    }
    MYSQL_RES* result = mysql_store_result(_conn);
    if(!result)
    {
        fprintf(stderr , "%s\n",mysql_error(_conn));
           return std::vector<Json::Value>();
    }
    int Hang = mysql_num_rows(result);
    std::vector<Json::Value> JGH; //JsonGroupHistory
    for(int i= 0;i<Hang;i++)
    {

         MYSQL_ROW Now_Hang = mysql_fetch_row(result);
         Json::Value history;

        Json::Reader Read;
        Read.parse(Now_Hang[2],history);
        history["user"] = Now_Hang[0];
         history["Time"] = Now_Hang[3] ;
        history["GroupId"] = Now_Hang[1];  //exprience:这一块测试时显示不全，总是少这三个，
                                            //记住：parse反序列化后再塞进时间辍之类的，不然就被覆盖掉了。
        JGH.push_back(history);
          
    }
    mysql_free_result(result);
    std::reverse(JGH.begin(), JGH.end());
    return JGH;
  }


bool MySqlClient::SaveOffline(Json::Value body,std::string username ,std::string friendname  ,std::string groupid)
{
    WriteItem item;
    item.body = body;
    item.username = username;
    item.friendname = friendname;
    item.groupid = groupid;
    item.offline = true;
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _writeQueue.push(item);
    }
    _queueCond.notify_one();
    return true;
}
std::vector<Json::Value> MySqlClient::PushOffline(std::string username)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::string order7 = "SELECT from_user , to_group,content , created_at FROM OFFLINE WHERE to_user = '"+username+"' ORDER BY id ASC";
        int judge7 = mysql_query(_conn , order7.c_str());
        if(judge7 != 0) exit(1);
        MYSQL_RES* result = mysql_store_result(_conn);
         if(NULL == result)
         {
            fprintf(stderr,"%s\n",mysql_error(_conn));
            return std::vector<Json::Value>();
         }
        my_ulonglong Hang = mysql_num_rows(result); //行
         std::vector<Json::Value> JPH;//JsonPrivateHistory

         for(int i = 0;i<Hang;i++)
        {
                Json::Value History;
            MYSQL_ROW NowHang = mysql_fetch_row(result); //这一行的数据都给他取出来
            Json::Reader Read;
           
            Read.parse(NowHang[2],History);
             History["User"] = NowHang[0];
             History["Group"] = NowHang[1];
            History["Time"] =NowHang[3];
           JPH.push_back(History);            
        }
        mysql_free_result(result);
        return JPH;

    }
bool MySqlClient::ClearOffline(std::string username)
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::string order_n = "DELETE FROM OFFLINE WHERE to_user = '"+username+"'";
    if(0 !=mysql_query(_conn,order_n.c_str()))
    {
        return false;
    }
    return true;   
}

void MySqlClient::writerLoop()
{
    while(_running)
    {
        std::vector<WriteItem> batch;
        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _queueCond.wait(lock, [this]{ return !_writeQueue.empty() || !_running; });
            while(!_writeQueue.empty() && batch.size() < 500)
            {
                batch.push_back(_writeQueue.front());
                _writeQueue.pop();
            }
        }
        if(batch.empty())
        {
            if(!_running) break;
            continue;
        }
        std::lock_guard<std::mutex> lock(_mutex);
        mysql_query(_conn, "BEGIN");
        for(const WriteItem& item : batch)
        {
            Json::StreamWriterBuilder builder;
            builder.settings_["emitUTF8"] = true;
            std::string JsonStr = Json::writeString(builder, item.body);
            std::string sql;
            if(item.offline)
                sql = "INSERT INTO OFFLINE(from_user,to_user,content , to_group) VALUES('" + Esc(_conn, item.username) +"','" + Esc(_conn, item.friendname)+"','"+Esc(_conn, JsonStr)+"', '"+ Esc(_conn, item.groupid)+ "')";
            else
                sql = "INSERT INTO ChatHistory(from_user,to_user,content,to_group) VALUES('"+ Esc(_conn, item.username) + "','" + Esc(_conn, item.friendname) + "','"+ Esc(_conn, JsonStr) + "','" + Esc(_conn, item.groupid) + "')";
            mysql_query(_conn, sql.c_str());
        }
        mysql_query(_conn, "COMMIT");
    }
}