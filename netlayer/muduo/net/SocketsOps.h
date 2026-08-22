#pragma once
#include <arpa/inet.h>

namespace muduo
{
namespace net
{
namespace sockets
{

int createNonblockingSockfd();
// void bind(int fd, const struct sockaddr* addr, socklen_t len);
// void listen(int fd);
// int accept(int fd, struct sockaddr* addr, socklen_t* len);
// void close(int fd);
// void shutdownWrite(int sockfd);
void setReuseAddr(int fd, bool on);
struct sockaddr_in getsockname(int sockfd);

}
}
}
