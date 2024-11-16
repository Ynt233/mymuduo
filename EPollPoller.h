/*
 * @Author: Ynt
 * @Date: 2024-11-14 12:52:56
 * @LastEditTime: 2024-11-16 15:20:33
 * @Description: 
 */
#pragma once
#include <vector>
#include <sys/epoll.h>
#include "Timestamp.h"
#include "Poller.h"

class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    void update(int operation, Channel *channel);

    using EventList = std::vector<epoll_event>;

    int epollfd_;
    EventList events_;
};
