#include "Client.h"
#include <iostream>
#include <thread>
#include <unistd.h>
#include <json/json.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/base/Logging.h>
#include "../BusinessLogic/MessageType.h"
#include <sstream>
#include <cstdlib>
#include "base64.h"
const int YMAXSENDBYTE = 65536;
void ySendFile(ChatClient* client , std::string yPath ,std::string to_somebody ,std::string groupname, int resumeOffset)
{
    size_t p = yPath.find_last_of('/');
    std::string y_filename = (p == std::string::npos)  ? yPath : yPath.substr(p+1);
    Json::Value JsData;
    JsData["filename"] = y_filename;
    JsData["to"]=to_somebody;
    JsData["token"] =client->getToken();
    JsData["group"] =groupname;
    JsData["resumeOffset"] = resumeOffset;
    FILE* ReadFile = fopen(yPath.c_str(), "rb");
    if(ReadFile == NULL)
    {
        std::cout << "[file] 文件不存在或无法打开: " << yPath << std::endl;
        return;   // 文件打不开，直接退出，别继续
    }
    fseek(ReadFile,0,SEEK_END);
    int yFileSize = ftell(ReadFile);
    JsData["filesize"] = yFileSize;
    client->send(100,JsData);
    
    fseek(ReadFile, resumeOffset, SEEK_SET);
    int TotalSize = yFileSize - resumeOffset;
    char FileBuffer[YMAXSENDBYTE];
    int yMoveSize = resumeOffset;
    bool bOK = true;
    int batch = 0;
    while(1) 
    {
        int ySendSize = (TotalSize <=YMAXSENDBYTE) ? TotalSize : YMAXSENDBYTE;
        if(ySendSize <= 0) break;
        yMoveSize += ySendSize;
        fread(FileBuffer,sizeof(char),ySendSize,ReadFile);
        TotalSize = TotalSize - ySendSize;
        fseek(ReadFile,yMoveSize, SEEK_SET);
        
        JsData["data"] = base64::encode(std::string(FileBuffer,ySendSize));
        client->send(101,JsData);
        if(++batch % 10 == 0)
        {
            Json::Value hb;
            client->send(static_cast<uint16_t>(MessageType::HEARTBEAT), hb);
        }
        if(TotalSize <=0)break;
    }
    client ->send(102, JsData);
    fclose(ReadFile);
}
void ySendFileQuery(ChatClient* client, std::string yPath, std::string to_somebody, std::string groupname)
{
    client->setPendingPath(yPath);
    client->setPendingTo(to_somebody);
    client->setPendingGroup(groupname);
    if(groupname.empty())
    {
        Json::Value body;
        body["token"] = client->getToken();
        body["to"] = to_somebody;
        body["filename"] = (yPath.find_last_of('/')==std::string::npos)?yPath:yPath.substr(yPath.find_last_of('/')+1);
        client->send(103, body);
    }
    else
    {
        std::thread t([client, yPath, to_somebody, groupname]() 
        {
            ySendFile(client, yPath, to_somebody, groupname);
        });
        t.detach();
    }
}

int main(int argc, char* argv[])
{
    std::string ip = "127.0.0.1";   
    int port = 2026;                
    if(argc >= 2) ip = argv[1];      
    if(argc >= 3) port = std::atoi(argv[2]);  
    LOG_INFO << " pid = " << getpid();
    muduo::net::EventLoop loop;
    muduo::net::InetAddress Addr(ip,port);
    ChatClient client(&loop,Addr);
    client.start();
    std::thread stdinThread([&]()
    {
        std::string line;
        while(std::getline(std::cin , line))
        {
            if(line.empty()) continue;
            if(line[0] == '/')  
            {
                std::istringstream iss(line);
                std::string cmd;
                iss >> cmd;
                //命令
                if(cmd == "/register")
                {
                    std::string user, pass, email, code;
                    iss >> user >> pass >> email >> code;
                    Json::Value body;
                    body["username"] = user;
                    body["password"] = pass;
                    body["email"] = email;
                    body["code"] = code;
                    loop.runInLoop([&client,body]()
                    {
                        client.send(666,body);
                    });
                }
                if(cmd == "/getcode")
                {
                    std::string email;
                    iss >> email;
                    Json::Value body;
                    body["email"] = email;
                    loop.runInLoop([&client, body]()
                    {
                        client.send(105, body);
                    });
                }
                if(cmd == "/resetpwd")
                {
                    std::string user, email, code, newpass;
                    iss >> user >> email >> code >> newpass;
                    Json::Value body;
                    body["username"] = user;
                    body["email"] = email;
                    body["code"] = code;
                    body["newpass"] = newpass;
                    loop.runInLoop([&client,body]()
                    {
                        client.send(106,body);
                    });
                }
                if(cmd == "/login")
                {
                    std::string user , pass;
                    iss >> user >> pass;
                    Json::Value body;
                    body["username"] = user;
                    body["password"] = pass;
                    loop.runInLoop([&client , body]() {client.send(66,body); });
                }
                if(cmd == "/login_code")
                {
                    std::string user, code;
                    iss >> user >> code;
                    Json::Value body;
                    body["username"] = user;
                    body["code"] = code;
                    loop.runInLoop([&client,body]()
                    {
                        client.send(109,body);
                    });
                }
                if(cmd == "/add")
                {
                    std::string friendname;
                    iss >> friendname;
                    Json::Value body;
                    body["token"] = client.getToken();
                    body["friend"] = friendname;
                    loop.runInLoop([&client , body]()
                    {
                        client.send(86,body);
                    });
                }
                if(cmd == "/yes")
                {
                    std::string friendname;
                    iss >> friendname;
                    Json::Value body;
                    body["token"] = client.getToken();
                    body["friend"] = friendname;
                    loop.runInLoop([&client,body]()
                    {
                        client.send(107,body);
                    });
                }
                if(cmd == "/no")
                {
                    std::string friendname;
                    iss >> friendname;
                    Json::Value body;
                    body["token"] = client.getToken();
                    body["friend"] = friendname;
                    loop.runInLoop([&client,body]()
                    {
                        client.send(108,body);
                    });
                }
                if(cmd == "/msg")
                {
                    std::string to , text;
                    iss >> to;
                    std::getline(iss , text);
                    Json::Value body;
                    body["token"] = client.getToken();
                    body["to"] = to;
                    body["msg"] = text;
                    loop.runInLoop([&client , body]()
                    {
                        client.send(729,body);
                    });
                }
                if(cmd == "/ai")
                {
                    std::string question;
                    std::getline(iss,question);
                    if(question.empty())
                    {
                        std::cout << "[!]请你带上问题";
                        continue;
                    }
                    Json::Value body;
                    body["token"] = client.getToken();
                    body["to"] = "AI_bot";
                    body["msg"] = question;
                    loop.runInLoop([&client,body]() {client.send(729,body);});
                }
                if(cmd == "/logout")
                {
                    Json::Value body;
                    body["token"] = client.getToken();
                    loop.runInLoop([&client,body]()
                {
                    client.send(68,body);
                });
                }
                if(cmd == "/deleteaccount")
                {
                    Json::Value body;
                    body["token"] = client.getToken();
                    loop.runInLoop([&client,body]()
                     {
                      client.send(69,body);
                    });
                }
                if(cmd == "/friends")
                {
                    Json::Value body;
                    body["token"] = client.getToken();
                    loop.runInLoop([&client , body]()
                    {
                        client.send(2026,body);
                    });
                }
                if(cmd == "/delete")
                {
                    std::string name;
                    iss >> name;
                    Json::Value body;
                    body["token"] = client.getToken();
                    body["friend"] = name;
                    loop.runInLoop([&client , body]() 
                    {
                        client.send(2025,body);
                    });
                }
                if(cmd == "/block")
                {
                    std::string name;
                    iss >>name ;
                    Json::Value body ;
                    body["token"] = client.getToken();
                    body["friend"] = name;
                    loop.runInLoop([&client , body]()
                    {
                        client.send(2027,body);
                    });
                }
                if(cmd == "/unblock")
                {
                    std::string name;
                    iss >> name;
                    Json::Value body;
                    body["token"] = client.getToken();
                    body["friend"] = name;
                    loop.runInLoop([&client , body]()
                    {
                        client.send(2028,body);
                    });
                }
                if(cmd == "/history")
                {
                    std::string friendname;
                    int count = 300;
                    std::string groupname;
                    std::string countnumber;
                    iss >> friendname;
                    if(iss >> countnumber)
                    {
                        try { count = std::stoi(countnumber);}
                        catch(...) {}                              //...捕获所有异常
                    }
                    Json::Value body;
                    body["token"] = client.getToken();
                    body["friend"] = friendname;
                    body["group"] ="";
                    body["count"] = count;
                    loop.runInLoop([&client,body]()
                    {
                        client.send(811,body);
                    });
                }
                if(cmd == "/sendfile")
                {
                    std::string line2, to, path;
                    std::getline(iss, line2);     // 读整行剩余(可能含中文)
                    if(!line2.empty() && line2[0] == ' ')
                        line2.erase(0, 1);         // 去开头空格
                    size_t sp = line2.find(' ');   // 找第一个空格
                    if(sp == std::string::npos)
                    {
                        to = line2;                 // 只有接收方,没路径
                        path = "";
                    }
                    else
                    {
                        to = line2.substr(0, sp);   // 接收方
                        path = line2.substr(sp+1);  // 路径(剩余)
                    }
                    loop.runInLoop([&client, to, path]()
                    {
                        ySendFileQuery(&client, path, to , "");
                    });
                }
                if(cmd == "/group")
                {
                    std::string sub;
                    iss >> sub;
                    if(sub == "history")
                    {
                        std::string groupname;
                        iss >> groupname;
                        Json::Value body;
                        body["token"] = client.getToken();
                        body["group"] = groupname;
                        body["friend"] = "";
                        loop.runInLoop([&client ,body]()
                        {
                            client.send(811 ,body);
                        });
                    }else if(sub == "create")
                     {
                         std::string groupname;
                         iss >> groupname;
                         Json::Value body;
                         body["token"] = client.getToken();
                         body["group"] = groupname;
                         loop.runInLoop([&client , body]()
                         {
                             client.send(1950 , body);
                         });
                     }
                     else if(sub == "join")
                    {
                        std::string groupname;
                        iss >> groupname;
                        Json::Value body;
                        body["token"] = client.getToken();
                        body["group"] = groupname;
                        loop.runInLoop([&client , body]()
                        {
                            client.send(2007 , body);
                        });
                    }
                      else if(sub == "delete")
                    {
                        std::string groupname;
                        iss >> groupname;
                        Json::Value body;
                        body["token"] = client.getToken();
                        body["group"] = groupname;
                        loop.runInLoop([&client , body]()
                        {
                            client.send(14,body);
                        });
                    }
                    else if(sub == "leave")
                    {
                        std::string groupname;
                        iss >> groupname;
                        Json::Value body;
                        body["token"] = client.getToken();
                        body["group"] = groupname;
                        loop.runInLoop([&client , body]()
                        {
                            client.send(15 , body);
                        });
                    }
                    else if(sub == "members")
                    {
                        std::string groupname;
                        iss >> groupname;
                        Json::Value body;
                        body["token"] = client.getToken();
                        body["group"] = groupname;
                        loop.runInLoop([&client , body]()
                        {
                            client.send(16 , body);
                        });
                     }
                    else if(sub == "my")
                    {
                        Json::Value body;
                        body["token"] = client.getToken();
                        loop.runInLoop([&client , body]()
                        {
                            client.send(17 , body);
                        });
                    }else if(sub == "addadmin")
                    {
                        std::string groupname, target;
                        iss >> groupname >> target;
                        Json::Value body;
                        body["token"] = client.getToken();
                        body["group"] = groupname;
                        body["target"] = target;
                        loop.runInLoop([&client , body]()
                        {
                            client.send(12 , body);
                        });
                    }
                    else if(sub == "removeadmin")
                    {
                        std::string groupname, target;
                        iss >> groupname >> target;
                        Json::Value body;
                        body["token"] = client.getToken();
                        body["group"] = groupname;
                        body["target"] = target;
                        loop.runInLoop([&client , body]()
                        {
                            client.send(19 , body);
                        });
                    }
                    else if(sub == "approve")
                    {
                        std::string groupname, target;
                        iss >> groupname >> target;
                        Json::Value body;
                        body["token"] = client.getToken();
                        body["group"] = groupname;
                        body["target"] = target;
                        loop.runInLoop([&client , body]()
                        {
                            client.send(13 , body);
                        });
                    }else if(sub == "kick")
                    {
                        std::string groupname, target;
                        iss >> groupname >> target;
                        Json::Value body;
                        body["token"] = client.getToken();
                        body["group"] = groupname;
                        body["target"] = target;
                        loop.runInLoop([&client , body]()
                        {
                            client.send(18 , body);
                        });
                    }
                    else if(sub == "msg")
                    {
                        std::string groupname, text;
                        iss >> groupname;
                        std::getline(iss , text);
                        Json::Value body;
                        body["token"] = client.getToken();
                        body["group"] = groupname;
                        body["msg"] = text;
                        loop.runInLoop([&client , body]()
                        {
                            client.send(06 , body);
                        });
                    }
                    else if(sub== "sendfile")
                    {
                        std::string groupname, path;
                        iss >> groupname; 
                        iss>> path;
                        loop.runInLoop([&client, groupname, path]()
                        {
                            ySendFileQuery(&client, path, "", groupname);
                        });
                    }
                }

                    
            }
            else
            {
                //普通
                Json::Value body;
                body["msg"] = line;
                loop.runInLoop([&client , body]()
                {
                    client.send(1,body);
                });
            }
        }
        loop.quit();
    });
    stdinThread.detach();
    loop.loop();
}