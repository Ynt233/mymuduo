/*
 * @Author: Ynt
 * @Date: 2024-11-15 13:00:10
 * @LastEditTime: 2024-11-15 13:07:46
 * @Description: 
 */
#pragma once
#include <functional>
#include <mutex>
#include <condition_variable>
#include "Thread.h"
#include "noncopyable.h"

class EventLoop;

class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();

    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};