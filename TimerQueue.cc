/*
 * @Author: Ynt
 * @Date: 2025-02-09 15:41:17
 * @LastEditTime: 2025-02-09 18:32:30
 * @Description: 
 */
#include <sys/timerfd.h>
#include <unistd.h>
#include <cstring>

#include "TimerQueue.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"
#include "Logger.h"

int createTimerFd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_FATAL("Create timerfd failed\n");
    }
    return timerfd;
}

TimerQueue::TimerQueue(EventLoop* loop) 
    : loop_(loop),
      timerfd_(createTimerFd()),
      timerChannel_(loop, timerfd_),
      timers_(),
      callingExpiredTimers_(false)
{
    timerChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerChannel_.enableReading();
}

TimerQueue::~TimerQueue() 
{
    timerChannel_.disableAll();
    timerChannel_.remove();
    ::close(timerfd_);

    for (const TimerQueue::Entry& timer : timers_) {
        delete timer.second;
    }
}

struct timespec getTimetoExpire(Timestamp expiredTime)
{
    int64_t microseconds = expiredTime.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100) {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds%Timestamp::kMicroSecondsPerSecond)*1000);
    return ts;
}

void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t num;
    ssize_t n = ::read(timerfd, &num, sizeof(num));
    if (n != sizeof(num)) {
        LOG_ERROR("TimerQueue::handleRead() reads %d bytes instead of 8", (int)n);
    }
}

void resetTimer(int timerfd, Timestamp expiredTime)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, 0, sizeof(newValue));
    memset(&oldValue, 0, sizeof(oldValue));
    newValue.it_value = getTimetoExpire(expiredTime);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret) {
        LOG_ERROR("timerfd_settime error\n");
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp expiredTime, double interval)
{
    TimerPtr timer(new Timer(std::move(cb), expiredTime, interval));
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->getSeq());
}

void TimerQueue::addTimerInLoop(TimerPtr timer)
{
    bool isClostestToExpire = insert(timer);
    if (isClostestToExpire) {
        resetTimer(timerfd_, timer->getExpiredTime());
    }
}

bool TimerQueue::insert(TimerPtr timer)
{
    bool isClostestToExpire = false;
    Timestamp expiredTime = timer->getExpiredTime();
    TimerList::iterator iter = timers_.begin();
    if (iter == timers_.end() || expiredTime < iter->first) {
        isClostestToExpire = true;
    }
    timers_.insert(Entry(expiredTime, timer));
    activeTimers_.insert(ActiveTimer(timer, timer->getSeq()));
    return isClostestToExpire;
}

void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(std::bind(&TimerQueue::cancelTimerInLoop, this, timerId));
}

void TimerQueue::cancelTimerInLoop(TimerId timerId)
{
    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    ActiveTimerList::iterator target = activeTimers_.find(timer);
    if (target != activeTimers_.end()) {
        size_t n = timers_.erase(Entry(target->first->getExpiredTime(), target->first));
        delete target->first;
        activeTimers_.erase(target);
    } else if (callingExpiredTimers_){
        cancelingTimers_.insert(timer);
    }
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry entry(now, reinterpret_cast<TimerPtr>(UINTPTR_MAX));
    TimerList::iterator end = timers_.lower_bound(entry);
    std::copy(timers_.begin(), end, std::back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    for (const Entry& en : expired) {
        ActiveTimer timer(en.second, en.second->getSeq());
        activeTimers_.erase(timer);
    }
    return expired;
}

void TimerQueue::reset(const std::vector<TimerQueue::Entry>& expired, Timestamp now)
{
    Timestamp nextExpiredTime;
    for (const Entry& en : expired) {
        ActiveTimer timer(en.second, en.second->getSeq());
        if (timer.first->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end()) {
            en.second->restart(now);
            insert(en.second);
        } else {
            delete en.second;
        }
    }

    if (!timers_.empty()) {
        nextExpiredTime = timers_.begin()->second->getExpiredTime();
    }
    if (nextExpiredTime.valid()) {
        resetTimer(timerfd_, nextExpiredTime);
    }
}

void TimerQueue::handleRead()
{
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);
    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    for (const Entry& en : expired) {
        en.second->run();
    }
    callingExpiredTimers_ = false;
    reset(expired, now);
}