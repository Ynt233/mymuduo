/*
 * @Author: Ynt
 * @Date: 2024-11-13 14:16:14
 * @LastEditTime: 2025-02-07 20:56:42
 * @Description: 
 */
#include <functional>
#include <string.h>
#include "Logger.h"
#include "TcpServer.h"

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr) {
        LOG_ERROR("%s:%s:%d main loop is null!\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

static sockaddr_in getLocalAddr(int sockfd)
{
    sockaddr_in localAddr;
    bzero(&localAddr, sizeof localAddr);
    socklen_t socklen = static_cast<socklen_t>(sizeof localAddr);
    if (::getsockname(sockfd, (sockaddr*)&localAddr, &socklen) < 0) {
        LOG_ERROR("InetAddress::getLocalAddr failed\n");
    }
    return localAddr;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &name, Option opt)
    : loop_(CheckLoopNotNull(loop)),
    ipPort_(listenAddr.toIpPort()),
    name_(name),
    acceptor_(new Acceptor(loop, listenAddr, opt == kReusePort)),
    threadPool_(new EventLoopThreadPool(loop, name)),
    connectionCallback_(),
    messageCallback_(),
    nextConnId_(1),
    started_(0)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
    
}

TcpServer::~TcpServer()
{
    for (auto &iter : connections_)
    {
        TcpConnectionPtr conn(iter.second); // use shared_ptr to automatically free
        iter.second.reset();

        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
    
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s\n", name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

    InetAddress localAddr(getLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
    // prevent starting one TcpServer for multiple times
    if (started_++ == 0) {
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection [%s]\n", name_.c_str(), conn->name().c_str());
    connections_.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}