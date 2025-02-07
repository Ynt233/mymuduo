/*
 * @Author: Ynt
 * @Date: 2024-11-13 14:21:37
 * @LastEditTime: 2024-11-16 16:26:17
 * @Description: Implementations of Channel class
 */

#include <sys/epoll.h>
#include <assert.h>
#include "Logger.h"
#include "EventLoop.h"
#include "Channel.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int sockfd)
    : loop_(loop), 
    fd_(sockfd), 
    events_(0), 
    revents_(0), 
    state_(-1), 
    tied_(false),
    eventHandling_(false),
    addedToLoop_(false)
{}

Channel::~Channel()
{
  assert(!eventHandling_);
  assert(!addedToLoop_);
}

void  Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

/*
* When changing events of fd in channel, updates the events corresponding to fd in poller
*/
void Channel::update() 
{
    // Call poller through the owning EventLoop, registering fd's events
    loop_->updateChannel(this);
}

/*
* Delete current channel from EventLoop
*/
void Channel::remove()
{
    loop_->removeChannel(this);
}

void Channel::eventHandler(Timestamp receiveTime)
{
    std::shared_ptr<void> guard;
    if (tied_) {
        guard = tie_.lock();
        if (guard) {
            eventHandlerWithGuard(receiveTime);
        }
    } else {
        eventHandlerWithGuard(receiveTime);
    }    
}

void Channel::eventHandlerWithGuard(Timestamp receiveTime)
{
    eventHandling_ = true;
    // LOG_DEBUG("channel eventHandler revents: %d", revents_);
    if (revents_ & EPOLLHUP && !(revents_ & EPOLLIN)) {
        if (closeCallback_) {
            closeCallback_();
        }  
    }
    
    if (revents_ & EPOLLERR) {
        if (errorCallback_) {
            errorCallback_();
        }  
    }

    if (revents_ & (EPOLLIN | EPOLLPRI)) {
        if (readEventCallback_) {
            readEventCallback_(receiveTime);
        }  
    }
    
    if (revents_ & EPOLLOUT) {
        if (writeCallback_) {
            writeCallback_();
        }  
    }
    
    eventHandling_ = false;
}