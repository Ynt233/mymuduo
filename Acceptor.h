/*
 * @Author: Ynt
 * @Date: 2024-11-15 14:02:48
 * @LastEditTime: 2024-11-15 14:48:15
 * @Description: 
 */
#pragma once
#include <functional>
#include "Socket.h"
#include "Channel.h"
#include "noncopyable.h"

class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop *loop, const InetAddress addr, bool reusePort);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb)
    {
        newConnectionCallback_ = std::move(cb);
    }

    bool listening() const { return listening_; }
    void listen();

private:
    void readHandler();

    EventLoop *loop_; // baseLoop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
};
