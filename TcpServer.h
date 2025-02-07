/*
 * @Author: Ynt
 * @Date: 2024-11-13 14:16:08
 * @LastEditTime: 2024-11-20 11:25:48
 * @Description: class exposed for server programming
 */

#pragma once
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>
#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "EventLoopThreadPool.h"
#include "Callback.h"
#include "Buffer.h"
#include "TcpConnection.h"
#include "noncopyable.h"

class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    
    enum Option
    {
        kNoReusePort,
        kReusePort
    };

    TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &name, Option opt = kNoReusePort);
    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

    void setThreadNum(int numThreads);

    EventLoop *getLoop() const { return loop_; }
    std::string name() const { return name_; }
    std::string ipPort() const { return ipPort_; }

    void start();
private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnecionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop *loop_; // baseLoop/mainLoop
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_; // run in mainLoop
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    
    ConnectionCallback connectionCallback_; // callback of receiving a new connection
    MessageCallback messageCallback_; // callback of receiving R/W event
    WriteCompleteCallback writeCompleteCallback_; // callback when message is sent

    ThreadInitCallback threadInitCallback_; // callback of initializing loop thread 
    std::atomic_int started_;

    int nextConnId_;
    ConnecionMap connections_; // save all tcp connections
};
