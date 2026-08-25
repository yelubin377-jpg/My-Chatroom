#include "Client.h"
#include <iostream>
#include <thread>
#include <unistd.h>
#include <termios.h>
#include <json/json.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/base/Logging.h>
#include "../BusinessLogic/MessageType.h"
#include <sstream>
#include <cstdlib>
#include "base64.h"
#include <chrono>
const int YMAXSENDBYTE = 65536;
void ySendFile(ChatClient* client , std::string yPath ,std::string to_somebody ,std::string groupname, int resumeOffset)
{
    size_t p = yPath.find_last_of('/');
    std::string y_filename = (p == std::string::npos)  ? yPath : yPath.substr(p+1);//npos表示没找到
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
        return;
    }
    fseek(ReadFile,0,SEEK_END);
    long yFileSize = ftell(ReadFile);
    JsData["filesize"] = yFileSize;
    JsData["fileId"] = y_filename + "_" + std::to_string(yFileSize);
    client->send(100,JsData);

    fseek(ReadFile, resumeOffset, SEEK_SET);
    long TotalSize = yFileSize - resumeOffset;
    char FileBuffer[YMAXSENDBYTE];
    long yMoveSize = resumeOffset;
    int lastPercent = -1;
    while(1)
    {
        int ySendSize = (TotalSize <= YMAXSENDBYTE) ? (int)TotalSize : YMAXSENDBYTE;
        if(ySendSize <= 0) break;
        yMoveSize += ySendSize;
        fread(FileBuffer,sizeof(char),ySendSize,ReadFile);
        TotalSize = TotalSize - ySendSize;
        JsData["data"] = base64::encode(std::string(FileBuffer,ySendSize));
        client->send(101,JsData);
        int percent = (yFileSize > 0) ? (int)(yMoveSize * 100 / yFileSize) : 100;
        if(percent != lastPercent)
        {
            lastPercent = percent;
            client->print("[发送] " + y_filename + " " + std::to_string(percent) + "%\n");
        }
        if(TotalSize <= 0) break;
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
int readChoice()
{
    std::string line;
    if(!std::getline(std::cin, line)) return -2;   // ctrl+D
    if(line.empty()) return -1;
    for(char c : line)
    {
        if(c < '0' || c > '9') return -1;   // 有非数字就非法
    }
    return std::atoi(line.c_str());
}

std::string readLine(const std::string& prompt)
{
    std::cout << prompt;
    std::cout.flush();
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_iflag |= IUTF8;                 // 关键:退格按整个中文字符删
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::string line;
    std::getline(std::cin, line);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return line;
}
std::string readPassword(const std::string& prompt)
{
    std::cout << prompt;
    std::cout.flush();
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);          // 先把终端当前设置存起来
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;                    // 关掉回显(真实字符不显示)
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // 应用
    std::string pass;
    char c;
    while(std::cin.get(c))
    {
        if(c == '\n' || c == '\r') break;     // 回车 = 输完
        if(c == 127 || c == '\b')             // 退格键
        {
            if(!pass.empty())
            {
                pass.pop_back();
                std::cout << "\b \b";         // 屏幕上抹掉一个 *
            }
        }
        else
        {
            pass.push_back(c);
            std::cout << '*';                 // 每输一位打一个 *
        }
        std::cout.flush();
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // 恢复回显
    std::cout << std::endl;
    return pass;
}

// ================== 菜单打印 ==================
void showMainMenu()
{
    std::cout <<
        "yChatRoom --- 登录菜单 --- yChatRoom\n"
        " 1. 登录\n"
        " 2. 注册\n"
        " 3. 获取邮箱验证码\n"
        " 4. 验证码登录\n"
        " 5. 找回密码\n"
        " 0. 退出\n"
        " 请选择: ";
}

void showUserMenu()
{
    std::cout <<
        "yChatRoom --- 主菜单 --- yChatRoom\n"
        " 1. 好友管理\n"
        " 2. 私聊\n"
        " 3. 群组\n"
        " 4. 聊天历史\n"
        " 5. 文件\n"
        " 6. 注销账号\n"
        " 7. 退出登录\n"
        " 0. 退出程序\n"
        " 请选择: ";
}

void showFriendMenu()
{
    std::cout <<
        "yChatRoom --- 好友管理 --- yChatRoom\n"
        " 1. 查看好友列表\n"
        " 2. 添加好友\n"
        " 3. 删除好友\n"
        " 4. 屏蔽好友\n"
        " 5. 取消屏蔽\n"
        " 6. 同意好友申请\n"
        " 7. 拒绝好友申请\n"
        " 8. 查看好友申请\n"
        " 0. 返回\n"
        " 请选择: ";
}

void showGroupMenu()
{
    std::cout <<
        "yChatRoom --- 群组 --- yChatRoom\n"
        " 1. 创建群\n"
        " 2. 加入群\n"
        " 3. 解散群\n"
        " 4. 退出群\n"
        " 5. 我的群列表\n"
        " 6. 查看群成员\n"
        " 7. 群聊天\n"
        " 8. 设置管理员\n"
        " 9. 取消管理员\n"
        " 10. 审批入群\n"
        " 11. 踢出成员\n"
        " 0. 返回\n"
        " 请选择: ";
}

void showFileMenu()
{
    std::cout <<
        "yChatRoom --- 文件 --- yChatRoom\n"
        " 1. 发送文件\n"
        " 2. 下载文件\n"
        " 0. 返回\n"
        " 请选择: ";
}

// ================== 登录等一级动作 ==================
bool doLogin(ChatClient* client, muduo::net::EventLoop* loop)
{
    std::string user = readLine(" 请输入用户名: ");
    Json::Value body;
    std::string pass = readPassword("请输入密码:");
    body["username"] = user;
    body["password"] = pass;
    client->resetLoginResult();
    loop->runInLoop([client, body]() { client->send(66, body); });
    while(!client->loginDone())
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return client->loginOk();
}

bool doLoginCode(ChatClient* client, muduo::net::EventLoop* loop)
{
    std::string user = readLine(" 请输入用户名: ");
    std::string code = readLine(" 请输入验证码: ");
    Json::Value body;
    body["username"] = user;
    body["code"] = code;
    client->resetLoginResult();
    loop->runInLoop([client, body]() { client->send(109, body); });
    while(!client->loginDone())
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return client->loginOk();
}

void doRegister(ChatClient* client, muduo::net::EventLoop* loop)
{
    std::string user = readLine(" 请输入用户名: ");
    std::string pass = readPassword(" 请输入密码: ");
    std::string email = readLine(" 请输入邮箱: ");
    std::string code = readLine(" 请输入验证码: ");
    Json::Value body;
    body["username"] = user;
    body["password"] = pass;
    body["email"] = email;
    body["code"] = code;
    loop->runInLoop([client, body]() { client->send(666, body); });
}

void doGetCode(ChatClient* client, muduo::net::EventLoop* loop)
{
    std::string email = readLine(" 请输入邮箱: ");
    Json::Value body;
    body["email"] = email;
    loop->runInLoop([client, body]() { client->send(105, body); });
}

void doResetPwd(ChatClient* client, muduo::net::EventLoop* loop)
{
    std::string user = readLine(" 请输入用户名: ");
    std::string email = readLine(" 请输入邮箱: ");
    std::string code = readLine(" 请输入验证码: ");
    std::string newpass = readPassword(" 请输入新密码: ");
    Json::Value body;
    body["username"] = user;
    body["email"] = email;
    body["code"] = code;
    body["newpass"] = newpass;
    loop->runInLoop([client, body]() { client->send(106, body); });
}

// ================== 查列表缓存 ==================
const std::vector<std::string>& fetchFriends(ChatClient* client, muduo::net::EventLoop* loop)
{
    client->resetFriends();
    Json::Value body;
    body["token"] = client->getToken();
    loop->runInLoop([client, body]() { client->send(2026, body); });
    while(!client->friendsDone())
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return client->friendsCache();
}

const std::vector<std::string>& fetchGroups(ChatClient* client, muduo::net::EventLoop* loop)
{
    client->resetGroups();
    Json::Value body;
    body["token"] = client->getToken();
    loop->runInLoop([client, body]() { client->send(17, body); });
    while(!client->groupsDone())
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return client->groupsCache();
}

// ================== 会话循环(一直发, /q 退出) ==================
void privateSession(ChatClient* client, muduo::net::EventLoop* loop, const std::string& to)
{
    std::cout << "yChatRoom --- 与 " << to << " 私聊中 --- yChatRoom\n";
    std::cout << "(支持换行，空行回车发送, 输入 /q 返回)\n";
    std::string all;
    std::string line;
    while(std::getline(std::cin, line))
    {
        if(line == "/q")
        {
            if(!all.empty())
            {
                Json::Value body;
                body["token"] = client->getToken();
                body["to"] = to;
                body["msg"] = all;
                std::thread t([client, body]() { client->send(729, body); });
                t.detach();
            }
            break;
        }
        if(line.empty())
        {
            if(!all.empty())
            {
                Json::Value body;
                body["token"] = client->getToken();
                body["to"] = to;
                body["msg"] = all;
                std::thread t([client, body]() { client->send(729, body); });
                t.detach();
                all.clear();
            }
        }
        else
        {
            if(!all.empty()) all += "\n";
            all += line;
        }
    }
}

void groupSession(ChatClient* client, muduo::net::EventLoop* loop, const std::string& group)
{
    std::cout << "yChatRoom --- 群 [" << group << "] 聊天中 --- yChatRoom\n";
    std::cout << "(支持换行，空行回车发送, 输入 /q 返回)\n";
    std::string all;
    std::string line;
    while(std::getline(std::cin, line))
    {
        if(line == "/q")
        {
            if(!all.empty())
            {
                Json::Value body;
                body["token"] = client->getToken();
                body["group"] = group;
                body["msg"] = all;
                std::thread t([client, body]() { client->send(6, body); });
                t.detach();
            }
            break;
        }
        if(line.empty())
        {
            if(!all.empty())
            {
                Json::Value body;
                body["token"] = client->getToken();
                body["group"] = group;
                body["msg"] = all;
                std::thread t([client, body]() { client->send(6, body); });
                t.detach();
                all.clear();
            }
        }
        else
        {
            if(!all.empty()) all += "\n";
            all += line;
        }
    }
}




// ================== 选对象再聊 ==================
void pickFriendThenChat(ChatClient* client, muduo::net::EventLoop* loop)
{
    const std::vector<std::string>& list = fetchFriends(client, loop);
    if(list.empty())
    {
        std::cout << " 还没有好友, 先去添加吧\n";
        return;
    }
    std::cout << "yChatRoom --- 选择私聊对象 --- yChatRoom\n";
    for(size_t i = 0; i < list.size(); i++)
        std::cout << " " << (i+1) << ". " << list[i] << "\n";
    std::cout << " 0. 返回\n 请选择: ";
    int c = readChoice();
    if(c <= 0 || c > (int)list.size()) return;
    std::string to = list[c-1];
    size_t pos = to.find(" [");
    if(pos != std::string::npos) to = to.substr(0, pos);
    privateSession(client, loop, to);
}

void pickGroupThenChat(ChatClient* client, muduo::net::EventLoop* loop)
{
    const std::vector<std::string>& list = fetchGroups(client, loop);
    if(list.empty())
    {
        std::cout << " 还没加入任何群\n";
        return;
    }
    std::cout << "yChatRoom --- 选择群 --- yChatRoom\n";
    for(size_t i = 0; i < list.size(); i++)
    std::cout << " " << (i+1) << ". " << list[i] << "\n";
    std::cout << " 0. 返回\n 请选择: ";
    int c = readChoice();
    if(c <= 0 || c > (int)list.size()) return;
    groupSession(client, loop, list[c-1]);
}

// ================== 历史 ==================
void doHistory(ChatClient* client, muduo::net::EventLoop* loop)
{
    std::cout << " 1. 私聊历史  2. 群聊历史\n 请选择: ";
    int t = readChoice();
    std::string name = readLine(" 请输入对方名或群名: ");
    if(name.empty())                              
    {
        std::cout << " 输入不能为空,已取消\n";
        return;
    }
    Json::Value body;
    body["token"] = client->getToken();
    body["count"] = 300;
    if(t == 2) { body["group"] = name; body["friend"] = ""; }
    else       { body["friend"] = name; body["group"] = ""; }
    loop->runInLoop([client, body]() { client->send(811, body); });
}

// ================== 子菜单循环 ==================
void friendLoop(ChatClient* client, muduo::net::EventLoop* loop)
{
    while(true)
    {
        showFriendMenu();
        int c = readChoice();
        if(c == 0 || c == -2) return;

        if(c == 1)
        {
            const std::vector<std::string>& list = fetchFriends(client, loop);
            for(size_t i = 0; i < list.size(); i++)
                std::cout << " " << (i+1) << ". " << list[i] << "\n";
        }
        else if(c >= 2 && c <= 5)
        {
            std::string name = readLine(" 请输入好友用户名: ");
            int type = 0;
            if(c == 2) type = 86;
            else if(c == 3) type = 2025;
            else if(c == 4) type = 2027;
            else if(c == 5) type = 2028;
            Json::Value body;
            body["token"] = client->getToken();
            body["friend"] = name;
            loop->runInLoop([client, body, type]() { client->send(type, body); });
        }
        else if(c == 6 || c == 7)
        {
            std::string name = readLine(" 请输入申请人用户名: ");
            Json::Value body;
            body["token"] = client->getToken();
            body["friend"] = name;
            int type = (c == 6) ? 107 : 108;
            loop->runInLoop([client, body, type]() { client->send(type, body); });
        }
        else if(c == 8)
        {
            Json::Value body;
            body["token"] = client->getToken();
            loop->runInLoop([client, body]() { client->send(112, body); });
        }
        else
            std::cout << " 无效选项\n";
    }
}

void groupLoop(ChatClient* client, muduo::net::EventLoop* loop)
{
    while(true)
    {
        showGroupMenu();
        int c = readChoice();
        if(c == 0 || c == -2) return;

        if(c == 1)      // 创建群
        {
            std::string g = readLine(" 请输入群名: ");
            Json::Value members;
            while(true)
            {
                std::string m = readLine(" 请输入初始成员用户名(至少2个,空行结束): ");
                if(m.empty()) break;
                members.append(m);
            }
            if(members.size() < 2)
            {
                std::cout << " 建群至少需要3人(你+2个好友),已取消\n";
                continue;
            }
            Json::Value body; 
            body["token"] = client->getToken(); 
            body["group"] = g; 
            body["members"] = members;
            loop->runInLoop([client, body]() { client->send(1950, body); });
        }
        else if(c == 2) // 加入群
        {
            std::string g = readLine(" 请输入群名: ");
            Json::Value body; body["token"] = client->getToken(); body["group"] = g;
            loop->runInLoop([client, body]() { client->send(2007, body); });
        }
        else if(c == 3) // 解散群
        {
            std::string g = readLine(" 请输入群名: ");
            Json::Value body; body["token"] = client->getToken(); body["group"] = g;
            loop->runInLoop([client, body]() { client->send(14, body); });
        }
        else if(c == 4) // 退出群
        {
            std::string g = readLine(" 请输入群名: ");
            Json::Value body; body["token"] = client->getToken(); body["group"] = g;
            loop->runInLoop([client, body]() { client->send(15, body); });
        }
        else if(c == 5) // 我的群列表
        {
            const std::vector<std::string>& list = fetchGroups(client, loop);
            for(size_t i = 0; i < list.size(); i++)
                std::cout << " " << (i+1) << ". " << list[i] << "\n";
        }
        else if(c == 6) // 查看群成员
        {
            std::string g = readLine(" 请输入群名: ");
            Json::Value body; body["token"] = client->getToken(); body["group"] = g;
            loop->runInLoop([client, body]() { client->send(16, body); });
        }
        else if(c == 7) // 群聊天
        {
            pickGroupThenChat(client, loop);
        }
        else if(c == 8 || c == 9 || c == 10 || c == 11) // 管理员/审批/踢人, 都要群名+目标
        {
            std::string g = readLine(" 请输入群名: ");
            std::string t = readLine(" 请输入目标成员: ");
            int type = 0;
            if(c == 9) type = 12;
            else if(c == 10) type = 19;
            else if(c == 11) type = 13;
            else if(c == 12) type = 18;
            Json::Value body; body["token"] = client->getToken(); body["group"] = g; body["target"] = t;
            loop->runInLoop([client, body, type]() { client->send(type, body); });
        }
        else
            std::cout << " 无效选项\n";
    }
}
void downloadFile(ChatClient* client, muduo::net::EventLoop* loop)
{
    client->resetFiles();
    Json::Value body;
    body["token"] = client->getToken();
    loop->runInLoop([client, body]() { client->send(110, body); });
    while(!client->filesDone())
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    const std::vector<std::string>& list = client->filesCache();
    if(list.empty())
    {
        std::cout << " 还没有可下载的文件\n";
        return;
    }
    std::cout << "yChatRoom --- 下载文件 --- yChatRoom\n";
    for(size_t i = 0; i < list.size(); i++)
    {
        size_t bar = list[i].find('|');
        std::string name = (bar == std::string::npos) ? list[i] : list[i].substr(bar + 1);
        std::cout << " " << (i+1) << ". " << name << "\n";
    }
    std::cout << " 0. 返回\n 请选择: ";
    int c = readChoice();
    if(c <= 0 || c > (int)list.size()) return;
    std::string entry = list[c-1];
    size_t bar = entry.find('|');
    std::string fileId = entry.substr(0, bar);
    std::string rest = entry.substr(bar + 1);
    size_t bar2 = rest.find('|');
    std::string filename = (bar2 == std::string::npos) ? rest : rest.substr(0, bar2);
    std::string from = (bar2 == std::string::npos) ? "" : rest.substr(bar2 + 1);

    long offset = 0;
    FILE* pf = fopen((fileId + ".part").c_str(), "rb");
    if(pf)
    {
        fseek(pf, 0, SEEK_END);
        offset = ftell(pf);
        fclose(pf);
    }

    Json::Value dl;
    dl["token"] = client->getToken();
    dl["fileId"] = fileId;
    dl["filename"] = filename;
    dl["offset"] = offset;
    dl["from"] = from;
    loop->runInLoop([client, dl]() { client->send(111, dl); });
    std::cout << (offset > 0 ? " 从 " + std::to_string(offset) + " 字节续传: " : " 开始下载: ") << filename << "\n";
}
void fileLoop(ChatClient* client, muduo::net::EventLoop* loop)
{
    while(true)
    {
        showFileMenu();
        int c = readChoice();
        if(c == 0 || c == -2) return;
        if(c == 1)
        {
            std::cout << " 1. 私聊发文件  2. 群聊发文件\n 请选择: ";
            int t = readChoice();
            if(t == 1)
            {
                std::string to = readLine(" 请输入对方用户名: ");
                std::string path = readLine(" 请输入文件路径: ");
                loop->runInLoop([client, to, path]() { ySendFileQuery(client, path, to, ""); });
            }
            else if(t == 2)
            {
                std::string group = readLine(" 请输入群名: ");
                std::string path = readLine(" 请输入文件路径: ");
                loop->runInLoop([client, group, path]() { ySendFileQuery(client, path, "", group); });
            }
        }
        else if(c == 2)
        {
            downloadFile(client, loop);
        }
        else
            std::cout << " 无效选项\n";
    }
}

// ================== 登录后主循环 ==================
void loggedInLoop(ChatClient* client, muduo::net::EventLoop* loop)
{
    while(true)
    {
        showUserMenu();
        int c = readChoice();
        if(c == -2 || c == 0) { _Exit(0);}
        if(c == 1) friendLoop(client, loop);
        else if(c == 2) pickFriendThenChat(client, loop);
        else if(c == 3) groupLoop(client, loop);
        else if(c == 4) doHistory(client, loop);
        else if(c == 5) fileLoop(client, loop);
        else if(c == 6) // 注销账号
        {
            Json::Value body; body["token"] = client->getToken();
            loop->runInLoop([client, body]() { client->send(69, body); });
            client->setToken("");
            std::cout << " 已注销账号\n";
            return;
        }
        else if(c == 7) // 退出登录
        {
            Json::Value body; body["token"] = client->getToken();
            loop->runInLoop([client, body]() { client->send(68, body); });
            client->setToken("");
            std::cout << " 已退出登录\n";
            return;
        }
        else
            std::cout << " 无效选项\n";
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
    muduo::net::InetAddress Addr(ip, port);
    ChatClient client(&loop, Addr);
    client.start();

    std::thread stdinThread([&]()
    {
        while(true)
        {
            showMainMenu();
            int c = readChoice();
            if(c == -2 || c == 0) { _Exit(0); }
            if(c == 1)
            {
                if(doLogin(&client, &loop)) loggedInLoop(&client, &loop);
            }
            else if(c == 2) doRegister(&client, &loop);
            else if(c == 3) doGetCode(&client, &loop);
            else if(c == 4)
            {
                if(doLoginCode(&client, &loop)) loggedInLoop(&client, &loop);
            }
            else if(c == 5) doResetPwd(&client, &loop);
            else std::cout << " 无效选项\n";
        }
    });
    stdinThread.detach();
    loop.loop();
}
