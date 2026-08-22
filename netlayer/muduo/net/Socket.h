#pragma once
#include <muduo/base/noncopyable.h>
#include "InetAddress.h"
#include <fcntl.h>
#include <unistd.h>

namespace muduo
{
namespace net
{

// noncopyable: fd不能拷贝，否则原fd关了之后副本还指着一块内核空间
// O_NONBLOCK + FD_CLOEXEC: 手动fcntl设，没用accept4

class Socket : noncopyable
{
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) 
    {}
    ~Socket() { ::close(sockfd_); }

    int fd() const { return sockfd_; }

    void bind(const InetAddress& addr)
    {
        ::bind(sockfd_, addr.getSockAddr(), addr.getSockAddrLen());
    }

    void listen()
    {
        ::listen(sockfd_, SOMAXCONN);
    }

    typedef struct
    {
        int socket;
        InetAddress peerAddr;
    } Accept_Result;

    Accept_Result accept()
    {
        sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        int connfd = ::accept(sockfd_, (sockaddr*)&addr, &addrlen);
        if (connfd < 0)
        {
            perror("accept");
            return { -1, InetAddress() };
        }
        int flags = ::fcntl(connfd, F_GETFL, 0);
        ::fcntl(connfd, F_SETFL, flags | O_NONBLOCK);
        ::fcntl(connfd, F_SETFD, FD_CLOEXEC);
        return { connfd, InetAddress(addr) };
    }

private:
    const int sockfd_;
};

}
}
