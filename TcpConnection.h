/*
 * @Author: Ynt
 * @Date: 2024-11-15 15:41:31
 * @LastEditTime: 2024-11-20 11:22:44
 * @Description: 
 */
#pragma once
#include <string>
#include <memory>
#include <any>
#include "Buffer.h"
#include "Callback.h"
#include "InetAddress.h"
#include "noncopyable.h"

class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop, const std::string& name, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }

    void send(const void* message, int len);
    void send(const std::string& message);

    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

    void setCloseCallback(const CloseCallback& cb)
    { closeCallback_ = cb; }

    void connectEstablished();
    void connectDestroyed();

    void setContext(const std::any& context)
    { context_ = context; }
    const std::any& getContext() const
    { return context_; }

    std::any* getMutableContext()
    { return &context_; }

private:
    enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected };

    void readHandler(Timestamp receiveTime);
    void writeHandler();
    void closeHandler();
    void errorHandler();

    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();

    void setState(StateE state) { state_ = state; }
    
    EventLoop *loop_; // subLoop
    const std::string name_;
    StateE state_;
    bool reading_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;
    Buffer inputBuffer_; // receive buffer
    Buffer outputBuffer_; // send buffer

    std::any context_;
};
