#pragma once
#include <arpa/inet.h>
#include <string>
#include <cstring>

namespace muduo
{
namespace net
{

class InetAddress
{
public:
    InetAddress()
    {
        memset(&addr_, 0, sizeof(addr_));
    }

    explicit InetAddress(const std::string& ip , uint16_t port)
    {
        memset(&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
    }

    explicit InetAddress(const struct sockaddr_in& addr)
        : addr_(addr)
    {}

    const struct sockaddr* getSockAddr() const
    {
        return reinterpret_cast<const struct sockaddr*>(&addr_);
    }

    socklen_t getSockAddrLen() const
    {
        return sizeof(addr_);
    }

    std::string toIp() const
    {
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
        return std::string(buf);
    }

    uint16_t toPort() const
    {
        return ntohs(addr_.sin_port);
    }

    std::string toIpPort() const
    {
        return toIp() + ":" + std::to_string(toPort());
    }

private:
    struct sockaddr_in addr_;
};

}
}
