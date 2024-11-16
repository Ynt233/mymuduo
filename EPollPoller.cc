/*
 * @Author: Ynt
 * @Date: 2024-11-14 12:53:04
 * @LastEditTime: 2024-11-16 16:26:16
 * @Description: 
 */
#include<errno.h>
#include <unistd.h>
#include <string.h>
#include "Logger.h"
#include "Channel.h"
#include "EPollPoller.h"

// channel hasn't been added to poller
const int kNew = -1;
// channel is added to poller
const int kAdded = 1;
// channel is deleted from poller
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop *loop) 
    : Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize)
{
    if (epollfd_ < 0) {
        LOG_FATAL("epoll create error: %d \n", errno);
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    LOG_DEBUG("func=%s => fd total count: %lu\n", __FUNCTION__, channels_.size());
    
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int err = errno;
    Timestamp now(Timestamp::now());

    if (numEvents > 0) {
        LOG_DEBUG("%d events happened\n", numEvents);
        fillActiveChannels(numEvents, activeChannels);

        // There could be events happened but epoll_wait didn't listen
        // So the events_ need to be expanded
        if (numEvents == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        LOG_DEBUG("%s timeout!\n", __FUNCTION__);
    } else {
        // EINTR means that a syscall is interrupted by a signal
        if (err != EINTR) {
            errno = err;
            LOG_ERROR("EPollPoller::poll() error: %d\n", err);
        }
    }
    return now;
}

void EPollPoller::updateChannel(Channel *channel)
{
    const int state = channel->state();
    // LOG_DEBUG("func=%s, fd=%d, event=%d, index=%d \n",__FUNCTION__, channel->fd(), channel->events(), state);
    if (state == kNew || state == kDeleted) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (state == kNew) {
            channels_[fd] = channel;
        }
        channel->set_state(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_state(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        } 
    }
}

// Remove channel from Poller
void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    int state = channel->state();

    channels_.erase(fd);

    if (state == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_state(kNew);
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(static_cast<int>(events_[i].events));
        // EventLoop gets a channel list(activeChannels) of all the occurred events that its poller returned to it
        activeChannels->push_back(channel);
    }
}

// epoll_ctl add/mod/del
void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    bzero(&event, sizeof event); // if use memset will cause core dump
    
    int fd = channel->fd();
    
    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;
    
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR("epoll_ctl del error: %d \n", errno);
        } else {
            LOG_FATAL("epoll_ctl add/mod error: %d \n", errno);
        }
    }
}