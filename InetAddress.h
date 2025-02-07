/*
 * @Author: Ynt
 * @Date: 2024-11-13 12:11:18
 * @LastEditTime: 2024-11-16 10:32:13
 * @Description: 
 */
#pragma once
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>

class InetAddress
{
public:
    explicit InetAddress(uint16_t port = 0, std::string ipAddr = "127.0.0.1", bool ipv6 = false);
    explicit InetAddress(const struct sockaddr_in& addr): addr_(addr) { }
    explicit InetAddress(const struct sockaddr_in6& addr): addr6_(addr) { }
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t port() const;

    bool isIpv6() const { return ipv6_; }
    
    const sockaddr_in* getSockAddr() const { return &addr_; };
    void setSockAddr6(const sockaddr_in6& addr6) { addr6_ = addr6; }

private:
    bool ipv6_ = false;
    union
    {
        sockaddr_in addr_;
        sockaddr_in6 addr6_;
    };
};


