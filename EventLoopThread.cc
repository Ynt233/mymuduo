/*
 * @Author: Ynt
 * @Date: 2024-11-15 13:00:19
 * @LastEditTime: 2024-11-15 13:27:15
 * @Description: 
 */
#include "EventLoop.h"
#include "EventLoopThread.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string& name)
    : loop_(nullptr),
    exiting_(false),
    thread_(std::bind(&EventLoopThread::threadFunc, this), name),
    mutex_(),
    cond_(),
    callback_(cb)
{}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    thread_.start(); // start new thread and execute EventLoopThread::threadFunc

    EventLoop *loop = nullptr;
    
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr) {
            cond_.wait(lock);
        }
        loop = loop_;
    }

    return loop;
}

// run in a seperate new thread created by start()
void EventLoopThread::threadFunc()
{
    EventLoop loop; // one loop per thread
    if (callback_) {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}