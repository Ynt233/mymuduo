/*
 * @Author: Ynt
 * @Date: 2024-11-14 12:17:43
 * @LastEditTime: 2024-11-14 12:52:42
 * @Description: 
 */
#include "Channel.h"
#include "Poller.h"

Poller::Poller(EventLoop *loop)
    : ownerLoop_(loop)
{
}

bool Poller::hashChannel(Channel *channel) const
{
    ChannelMap::const_iterator iter = channels_.find(channel->fd());
    return iter != channels_.end() && iter->second == channel;
}