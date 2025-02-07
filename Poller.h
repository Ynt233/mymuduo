/*
 * @Author: Ynt
 * @Date: 2024-11-14 12:17:37
 * @LastEditTime: 2024-11-16 12:00:20
 * @Description: Main IO multiplexing module
 */
#pragma once
#include <vector>
#include <unordered_map>
#include "noncopyable.h"

class Channel;
class EventLoop;
class Timestamp;

class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;
    
    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;
    
    // Return whether channel is in current poller
    bool hashChannel(Channel *channel) const;

    // Used by EventLoop to get default IO multiplexing implementation
    static Poller* newDefaultPoller(EventLoop *loop);

protected:
    // Key refers to sockfd, value refers to channel that sockfd belongs to
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;
private:
    EventLoop *ownerLoop_;
};

