/*
 * @Author: Ynt
 * @Date: 2024-11-15 13:28:02
 * @LastEditTime: 2025-02-08 00:37:19
 * @Description:
 */
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &name)
    : baseLoop_(baseLoop),
      name_(name),
      started_(false),
      numThreads_(0),
      next_(0)
{
}

// no need to delete loop, it is stack variable
EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    started_ = true;

    for (int i = 0; i < numThreads_; i++)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop()); // create a new thread and bind a new eventloop, return the address of this new loop
    }

    // only one thread in server, running baseLoop
    if (numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    EventLoop *loop = baseLoop_;
    if (!loops_.empty())
    {
        size_t minConnections = std::numeric_limits<size_t>::max(); // int_max
        for (const auto &curloop : loops_)
        {
            size_t activeChannelCount = curloop->getActiveChannelCount();
            if (activeChannelCount < minConnections)
            {
                minConnections = activeChannelCount;
                loop = curloop;
            }
        }
        // loop = loops_[next_];
        // ++next_;
        // if (next_ >= loops_.size())
        // {
        //     next_ = 0;
        // }
    }

    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
{
    if (loops_.empty())
    {
        return std::vector<EventLoop *>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}