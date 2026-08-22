#基本命令
cmake_minimum_required(VERSION 3.10)
project(ChatRoom)
set(CMAKE_CXX_STANDARD 17)  #是11还是17要注意,有概率是一克雷；

#告诉编译器去哪里找头文件和库
include_directories(/home/michael-377/build/release-install-cpp11/include)  #头文件路径
link_directories(/home/michael-377/build/release-install-cpp11/lib)  #静态库路径

#生成可执行文件
add_executable(chat_server     #生成的可执行文件
    main.cc             #编译这两个cpp文件
    hub.cc
)

#链接库
target_link_libraries(chat_server    #给 chat_server 这个可执行文件
    muduo_net                        #链接这三个库
    muduo_base
    pthread
)
    

