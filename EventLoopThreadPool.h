/*
 * @Author: Ynt
 * @Date: 2024-11-15 13:27:52
 * @LastEditTime: 2025-02-07 14:46:16
 * @Description:
 */
#pragma once
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include "noncopyable.h"
#include "EventLoop.h"

class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    EventLoopThreadPool(EventLoop *baseLoop, const std::string &name);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }

    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    // if it is multi-thread mode, baseLoop is the mainLoop that handle client connection and allocate channel to subLoop
    EventLoop *getNextLoop();

    std::vector<EventLoop *> getAllLoops();

    bool started() const { return started_; }
    const std::string name() const { return name_; }

private:
    EventLoop *baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};