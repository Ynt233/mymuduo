/*
 * @Author: Ynt
 * @Date: 2024-11-15 14:02:57
 * @LastEditTime: 2024-11-15 16:57:54
 * @Description: 
 */
#include <errno.h>
#include <unistd.h>
#include "Logger.h"
#include "Acceptor.h"

static int createNonblockingFd(bool isIpv6)
{
    int sockfd = -1;
    if (isIpv6) {
        sockfd = ::socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    } else {
        sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    }
    
    if (sockfd < 0) {
        LOG_FATAL("%s:%s:%d create listen socket error: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
    
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress addr, bool reusePort)
    : loop_(loop),
    acceptSocket_(createNonblockingFd(addr.isIpv6())),
    acceptChannel_(loop, acceptSocket_.fd()),
    listening_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(addr);

    // TcpServer::start() 
    acceptChannel_.setReadCallback(std::bind(&Acceptor::readHandler, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading(); // acceptChannel_ registers into poller
}

// a new client connect => new events happened in listenfd 
void Acceptor::readHandler()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr); // find subloop and wake it, allocate current channel to it
        } else {
            ::close(connfd);
        }
    } else {
        LOG_ERROR("%s:%s:%d accept error: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        if (errno == EMFILE) { // no resources in process
            LOG_ERROR("%s:%s:%d sockfd reached limit\n", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}