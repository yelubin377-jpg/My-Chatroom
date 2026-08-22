#include "SocketsOps.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <muduo/base/Logging.h>

using namespace muduo::net;

int sockets::createNonblockingSockfd()
{
    // 试过SOCK_NONBLOCK | SOCK_CLOEXEC直接传给socket()，但有些老内核不支持
    // 最后还是用fcntl，兼容性好点
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        LOG_ERROR << "sockets::createNonblockingSockfd failed";
        return fd;
    }

    int flags = ::fcntl(fd, F_GETFL, 0);               //fcntl 是个万能函数——对一个 fd 做各种控制。这里 F_GETFL = 读标志，F_SETFL = 写标志。
                                                        //fcntl 干两件不同的事，用两个不同命令区分：
                                                            // F_GETFL / F_SETFL  → 操作"文件状态标志"（像 O_NONBLOCK 这种影响 I/O 行为的）
                                                            // F_GETFD / F_SETFD  → 操作"文件描述符标志"（像 FD_CLOEXEC 这种影响 fd 本身生命周期的）
    ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);          //为什么分两步？ 因为不能直接写死标志——fd 上可能已经有别的标志了，直接覆盖会弄丢。所以先读出来（flags），再用 | 加上新的一位，再写回去。这跟 Channel 里 events_ |= EPOLLIN 是一个思路。
    ::fcntl(fd, F_SETFD, FD_CLOEXEC);   
    return fd;
}
//O_NONBLOCK = 非阻塞。这是 muduo 整个异步模型的根基：

//阻塞模式:  read(fd) → 没数据就一直卡着等，线程冻住
//非阻塞模式: read(fd) → 没数据立刻返回 -1，你可以去干别的

//如果 socket 是阻塞的，那么 epoll 说"可读"之后你去 read，理论上数据已经到了不会卡。但网络有边界情况（比如数据被对端抢走），所以 muduo 强制所有 fd 非阻塞——保证任何 read/write 都不会卡住事件循环。



//合格的非阻塞 socket
//① F_GETFL 读标志        → 拿现有状态
//② F_SETFL | O_NONBLOCK  → 加"非阻塞"这一位
//③ F_SETFD FD_CLOEXEC    → 加"exec 自动关闭"保护
 


//EAGAIN（非阻塞的核心信号）
// 这个最关键，单独说。
// // 非阻塞 read，此时恰好没数据
// int n = ::read(fd, buf, len);
// // n = -1，errno = EAGAIN
// EAGAIN 的字面意思就是 "try again"（再试一次），但在这里实际含义是："现在没有数据可读，不是错误，你别卡着等，回去干别的。"



// void sockets::bind(int fd, const struct sockaddr* addr, socklen_t len)
// {
//     if (::bind(fd, addr, len) < 0)
//     {
//         LOG_ERROR << "sockets::bind failed";
//     }
// }

// void sockets::listen(int fd)
// {
//     if (::listen(fd, SOMAXCONN) < 0)
//     {
//         LOG_ERROR << "sockets::listen failed";
//     }
// }

// int sockets::accept(int fd, struct sockaddr* addr, socklen_t* len)
// {
//     int connfd = ::accept(fd, addr, len);
//     if (connfd >= 0)
//     {
//         int flags = ::fcntl(connfd, F_GETFL, 0);
//         ::fcntl(connfd, F_SETFL, flags | O_NONBLOCK);
//         ::fcntl(connfd, F_SETFD, FD_CLOEXEC);
//     }
//     else
//     {
//         LOG_ERROR << "sockets::accept failed";
//     }
//     return connfd;
// }

// void sockets::shutdownWrite(int sockfd)
// {
//     ::shutdown(sockfd, SHUT_WR);
// }

// void sockets::close(int fd)
// {
//     if (::close(fd) < 0)
//     {
//         LOG_ERROR << "sockets::close failed";
//     }
// }

void sockets::setReuseAddr(int fd, bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

struct sockaddr_in sockets::getsockname(int sockfd)  //拿地址
{
    struct sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);
    ::getsockname(sockfd, (struct sockaddr*)&localaddr, &addrlen);
    return localaddr;
}


//  struct sockaddr_in 
//   {
//       sa_family_t    sin_family;   // 地址族，AF_INET = IPv4
//       uint16_t       sin_port;     // 端口号（网络字节序）
//       struct in_addr sin_addr;     // IP 地址
//       char           sin_zero[8];  // 补齐用的，无实际意义
//   };
