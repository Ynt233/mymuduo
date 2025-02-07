/*
 * @Author: Ynt
 * @Date: 2024-11-13 14:16:58
 * @LastEditTime: 2024-11-20 11:09:16
 * @Description: Contains two main modules: Channel, Poller(abstract of epoll)
 * Can be understood as the demultiplex part in Reactor model
 */

#pragma once
#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>
#include <any>
#include "CurrentThread.h"
#include "Timestamp.h"
#include "noncopyable.h"

class Channel;
class Poller;

class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    // start event loop
    void loop();
    // stop event loop
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    // execute callback in current loop
    void runInLoop(Functor cb);
    // put callback into queue, wake up the thread where the loop settles to execute callback
    void queueInLoop(Functor cb);
    void wakeup();

    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    void hashChannel(Channel *channel);

    // Return if it is in the thread that created it
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

    void setContext(const std::any& context)
    { context_ = context; }

    const std::any& getContext() const
    { return context_; }

    std::any* getMutableContext()
    { return &context_; }
    
private:
    void readHandler(); // wake up
    void doPendingFunctors();

    using ChannelList = std::vector<Channel*>;
    
    std::atomic_bool looping_; //atomic
    std::atomic_bool quit_; // marks quiting loop
    
    const pid_t threadId_; // current thread id
    Timestamp pollReturnTime_; // timing that poller return activeChannels list
    std::unique_ptr<Poller> poller_;

    int wakeupFd_; // when mainLoop gets a new user channel, select a subloop by using polling algorithm and wake it up 
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;

    std::atomic_bool callingPendingFunctors_; // marks if current loop has callback that needs to be done
    std::vector<Functor> pendingFunctors_; // saves all callbacks that loop needs to perform
    std::mutex mutex_; // exclusion lock, protects thread-safe operations of the pendingFunctors_

    std::any context_;
};

