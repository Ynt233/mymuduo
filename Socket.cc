/*
 * @Author: Ynt
 * @Date: 2024-11-15 14:03:44
 * @LastEditTime: 2024-11-16 15:58:38
 * @Description: 
 */
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "Logger.h"
#include "Socket.h"

Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress &localAddr)
{
    int ret = ::bind(sockfd_, (sockaddr *)localAddr.getSockAddr(), sizeof(sockaddr_in6));
    if (ret != 0) {
        LOG_FATAL("bind sockfd: %d failed\n", sockfd_);
    }
}

void Socket::listen()
{
    int ret = ::listen(sockfd_, 1024);
    if (ret != 0) {
        LOG_FATAL("listen sockfd: %d failed\n", sockfd_);
    }
}

int Socket::accept(InetAddress *peerAddr)
{
    sockaddr_in6 addr;
    socklen_t len = sizeof addr;
    bzero(&addr, sizeof addr);
    int connfd = ::accept4(sockfd_, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0) {
        peerAddr->setSockAddr6(addr);
    }
    return connfd;
}

void Socket::shutdownRead()
{
    int ret = ::shutdown(sockfd_, SHUT_RD);
    if (ret < 0) {
        LOG_ERROR("shutdown write error: %d", errno);
    }
}

void Socket::shutdownWrite()
{
    int ret = ::shutdown(sockfd_, SHUT_WR);
    if (ret < 0) {
        LOG_ERROR("shutdown write error: %d", errno);
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof opt);
}

void Socket::setReuseAddr(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
}

void Socket::setReusePort(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof opt);
}

void Socket::setKeepAlive(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof opt);
}