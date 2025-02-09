/*
 * @Author: Ynt
 * @Date: 2025-02-09 15:04:19
 * @LastEditTime: 2025-02-09 17:44:18
 * @Description: 
 */

#pragma once
#include <memory>

class Timer;

class TimerId
{
public:
    friend class TimerQueue;
    TimerId() : timer_(nullptr), sequence_(0) {}
    TimerId(Timer* timer, int64_t seq) : timer_(timer), sequence_(seq) {}

private:
    Timer* timer_;
    int64_t sequence_;
};
 