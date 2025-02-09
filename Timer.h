/*
 * @Author: Ynt
 * @Date: 2025-02-09 14:41:38
 * @LastEditTime: 2025-02-09 17:44:33
 * @Description: 
 */

 #pragma once

 #include <atomic>
 #include "noncopyable.h"
 #include "Callback.h"
 #include "Timestamp.h"

class Timer 
{
public:
    Timer(TimerCallback cb, Timestamp expiredTime, double interval = 0.0)
    : callback_(std::move(cb)),
      expiredTime_(expiredTime),
      interval_(interval),
      repeat_(interval > 0.0),
      sequence_(++numCreated_)
    {}

    void run() const { callback_(); }

    Timestamp getExpiredTime() const { return expiredTime_; }
    bool repeat() const { return repeat_;}
    int64_t getSeq() const { return sequence_; }
    static int64_t getNumCreated() { return numCreated_; }

    void restart(Timestamp now);
    
private:
    TimerCallback callback_;
    Timestamp expiredTime_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static std::atomic_int64_t numCreated_;
};