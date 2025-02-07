/*
 * @Author: Ynt
 * @Date: 2024-11-15 14:03:39
 * @LastEditTime: 2024-11-15 14:28:14
 * @Description: 
 */
#pragma once
#include "InetAddress.h"
#include "noncopyable.h"

class Socket : noncopyable
{
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();

    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress &localAddr);
    void listen();
    int accept(InetAddress *peerAddr);

    void shutdownRead();
    void shutdownWrite();
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
    
private:
    const int sockfd_;
};
