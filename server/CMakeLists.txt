cmake_minimum_required(VERSION 3.10)   #cmake 最低版本
project(ChatRoom)                        #项目名
set(CMAKE_CXX_STANDARD 17)              #用 C++17

include_directories(/usr/include/jsoncpp)
include_directories(/usr/include/hiredis)
include_directories(../netlayer)
add_executable(chat_server              # 生成的可执行文件名
    main.cc                             # server 目录下的
    ChatServer.cc
    Router.cc
    ../protocal/pack.cpp                # 跨目录引协议层
    ../protocal/unpack.cpp
    ../BusinessLogic/redisClient.cc               
    ../BusinessLogic/LoginHandler.cc
    ../BusinessLogic/RegisterHandler.cc
    ../BusinessLogic/LogoutHandler.cc
    ../BusinessLogic/DeleteAccountHandler.cc
    ../BusinessLogic/AddFriendHandler.cc
    ../BusinessLogic/DeleteFriendHandler.cc
    ../BusinessLogic/BlockFriendHandler.cc
    ../BusinessLogic/UnBlockFriendHandler.cc
    ../BusinessLogic/QueryFriendHandler.cc
    ../BusinessLogic/PrivateChatHandler.cc
    ../BusinessLogic/CreateGroupHandler.cc
    ../BusinessLogic/DeleteGroupHandler.cc
    ../BusinessLogic/JoinGroupHandler.cc
    ../BusinessLogic/LeaveGroupHandler.cc
    ../BusinessLogic/AcceptFriendHandler.cc      
    ../BusinessLogic/RejectFriendHandler.cc      
    ../BusinessLogic/GroupChatHandler.cc
    ../BusinessLogic/QueryGroupMembersHandler.cc
    ../BusinessLogic/QueryUserGroupsHandler.cc
    ../BusinessLogic/AddGroupAdminHandler.cc
    ../BusinessLogic/RemoveGroupAdminHandler.cc
    ../BusinessLogic/ApproveJoinHandler.cc
    ../BusinessLogic/RemoveGroupMembersHandler.cc  
    ../BusinessLogic/QueryHistoryHandler.cc
    ../BusinessLogic/QueryHistoryHandler.cc
    ../BusinessLogic/MySqlClient.cc   
    ../BusinessLogic/Hashtools.cc  
    ../BusinessLogic/FileTransferHandler.cc 
    ../netlayer/muduo/net/Acceptor.cc
    ../netlayer/muduo/net/Buffer.cc
    ../netlayer/muduo/net/Channel.cc
    ../netlayer/muduo/net/Connector.cc
    ../netlayer/muduo/net/EventLoop.cc
    ../netlayer/muduo/net/Poller.cc
    ../netlayer/muduo/net/SocketsOps.cc
    ../netlayer/muduo/net/TcpClient.cc
    ../netlayer/muduo/net/TcpConnection.cc
    ../netlayer/muduo/net/TcpServer.cc
    ../netlayer/muduo/net/TimerQueue.cc
    ../BusinessLogic/EmailSender.cc
    ../BusinessLogic/GetVerifyCodeHandler.cc
    ../netlayer/muduo/net/EventLoopThread.cc
    ../netlayer/muduo/net/EventLoopThreadPool.cc
    ../BusinessLogic/ResetPasswordHandler.cc
    ../BusinessLogic/LoginByCodeHandler.cc

)

target_link_libraries(chat_server       # 给 chat_server 链接
    jsoncpp                             # JSON 解析
    pthread                             # 多线程
    hiredis                             #redis
    curl
    mysqlclient     # MySQL 客户端库
    crypto    # OpenSSL 加密库，SHA256 在这
)