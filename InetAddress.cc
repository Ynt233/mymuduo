/*
 * @Author: Ynt
 * @Date: 2024-11-13 12:11:26
 * @LastEditTime: 2024-11-16 10:32:06
 * @Description: Implementations of InetAddress class
 */
#include <strings.h>
#include <string.h>
#include "Logger.h"
#include "InetAddress.h"

InetAddress::InetAddress(uint16_t port, std::string ipAddr, bool ipv6)
{
    ipv6_ = ipv6;
    if (ipv6) {
        bzero(&addr6_, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_port = htons(port);
        ::inet_pton(AF_INET6, ipAddr.c_str(), &addr6_.sin6_addr);
    } else {
        bzero(&addr_, sizeof addr_);
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        addr_.sin_addr.s_addr = inet_addr(ipAddr.c_str());
    }
    
}

std::string InetAddress::toIp() const 
{   
    if (ipv6_) {
        char buf[64] = {0};
        ::inet_ntop(AF_INET6, &addr6_.sin6_addr, buf, sizeof buf);
        return buf;
    } else {
        char buf[64] = {0};
        ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
        return buf;
    }
}

std::string InetAddress::toIpPort() const 
{
    if (ipv6_) {
        char buf[64] = {0};
        ::inet_ntop(AF_INET6, &addr6_.sin6_addr, buf, sizeof buf);
        size_t end = strlen(buf); 
        uint16_t port = ntohs(addr6_.sin6_port);
        sprintf(buf + end, ":%u", port);
        return buf;
    } else {
        char buf[64] = {0};
        ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
        size_t end = strlen(buf); 
        uint16_t port = ntohs(addr_.sin_port);
        sprintf(buf + end, ":%u", port);
        return buf;
    } 
}

uint16_t InetAddress::port() const 
{
    if (ipv6_){
        return ntohs(addr6_.sin6_port);
    } else {
        return ntohs(addr_.sin_port);
    }
}