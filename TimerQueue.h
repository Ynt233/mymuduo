/*
 * @Author: Ynt
 * @Date: 2025-02-09 15:24:40
 * @LastEditTime: 2025-02-09 17:44:07
 * @Description: 
 */
#pragma once
#include <memory>
#include <vector>
#include <set>
#include <utility>

#include "noncopyable.h"
#include "Callback.h"
#include "Channel.h"
#include "Timestamp.h"

class Timer;
class TimerId;
class TimerQueue
{
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(TimerCallback cb, Timestamp expiredTime, double interval);
    void cancel(TimerId timerId);
private:
    using TimerPtr = Timer*;
    using Entry = std::pair<Timestamp, TimerPtr>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<TimerPtr, int64_t>;
    using ActiveTimerList = std::set<ActiveTimer>;

    void addTimerInLoop(TimerPtr timer);
    void cancelTimerInLoop(TimerId timerId);

    void handleRead();

    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(TimerPtr timer);

    EventLoop* loop_;
    const int timerfd_;
    Channel timerChannel_;
    TimerList timers_;
    ActiveTimerList activeTimers_;
    bool callingExpiredTimers_;
    ActiveTimerList cancelingTimers_;
};
