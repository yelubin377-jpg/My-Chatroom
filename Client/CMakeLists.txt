cmake_minimum_required(VERSION 3.10)
project(ChatRoomClient)
set(CMAKE_CXX_STANDARD 17)


include_directories(/usr/include/jsoncpp)
include_directories(../netlayer)

add_executable(chat_client
    main.cc
    Client.cc
    ../server/Router.cc
    ../protocal/pack.cpp
    ../protocal/unpack.cpp
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
    ../netlayer/muduo/net/EventLoopThread.cc
    ../netlayer/muduo/net/EventLoopThreadPool.cc
)

target_link_libraries(chat_client
    jsoncpp
    pthread
)