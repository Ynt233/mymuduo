/*
 * @Author: Ynt
 * @Date: 2024-11-13 14:17:04
 * @LastEditTime: 2025-02-09 17:27:29
 * @Description: 
 */
#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "Poller.h"
#include "Channel.h"
#include "Logger.h"
#include "EventLoop.h"

// prevents one thread from creating multiple EventLoop
__thread EventLoop *t_loopInThisThread = nullptr;

// default timeout of Poller IO multiplex
const int kPollTimeoutMs = 10000;

// create wakeup fd for notifying subReactor(subloop)
int createEventFd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_FATAL("eventfd error: %d\n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
    quit_(false),
    callingPendingFunctors_(false),
    threadId_(CurrentThread::tid()),
    poller_(Poller::newDefaultPoller(this)),
    wakeupFd_(createEventFd()),
    wakeupChannel_(new Channel(this, wakeupFd_)),
    timerQueue_(new TimerQueue(this))
{
    LOG_DEBUG("EventLoop created %p in thread %d\n", this, threadId_);
    if (t_loopInThisThread) {
        LOG_FATAL("Another EventLoop %p exists in this thread %d\n", t_loopInThisThread, threadId_);
    } else {
        t_loopInThisThread = this;
    }

    // set event and callbakck of wakeupfd
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::readHandler, this));
    // each eventloop will listen the EPOLLIN read events of wakeupchannel
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::readHandler()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR("EventLoop::readHandler() reads %lu bytes instead of 8\n",n);
    }
}

// start event loop
void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping\n", this);

    while (!quit_) {
        activeChannels_.clear();
        // listen to 2 kinds of fd: clientfd, wakeupfd
        pollReturnTime_ = poller_->poll(kPollTimeoutMs, &activeChannels_);
        for (Channel *channel : activeChannels_) {
            channel->eventHandler(pollReturnTime_);
        }
        // execute callbacks of current eventloop
        doPendingFunctors();
    }

    LOG_INFO("EventLoop %p stop looping\n", this);
    looping_ = false;
}

// stop event loop
void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread()) { // if quit() is called by other threads
        wakeup();
    }
}

// execute callback in current loop
void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread()) {
        cb();
    } else { // loop is running in another thread instead of its initial thread, needs to wakeup the correct thread  
        queueInLoop(cb);
    }
}

// put callback into queue, wake up the thread where the loop settles to execute callback
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    // wake up the corresponding thread to execute cb
    // line 117 has added a new cb, so if current thread is executing a cb, it also needs to be waked up.
    // otherwise, it will be blocked by poller_->poll()
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

// write data to wakeupfd, so wakeupChannel will read and current loop thread will be waked up
void EventLoop::wakeup()
{
    uint64_t one =1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR("EventLoop::wakeup() write %lu bytes instead of 8\n", n);
    }
}

void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

void EventLoop::hashChannel(Channel *channel)
{
    poller_->hashChannel(channel);
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    // clear the pendingFunctors_ and move the cbs into a temporary container: functors
    // because mainLoop may still adding cbs into pendingFunctors_, and it requires lock, which will cause a low performance
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor &functor : functors)
    {
        functor(); // execute current loop's cbs
    }
    
    callingPendingFunctors_ = false;
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb)
{
    return timerQueue_->addTimer(std::move(cb), time, 0.0);    
}


TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
    Timestamp expiredTime(addTime(Timestamp::now(), delay));
    return runAt(expiredTime, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
    Timestamp expiredTime(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(std::move(cb), expiredTime, interval);
}

void EventLoop::cancelTimer(TimerId timerId)
{
    return timerQueue_->cancel(timerId);
}