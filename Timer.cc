/*
 * @Author: Ynt
 * @Date: 2025-02-09 14:58:36
 * @LastEditTime: 2025-02-09 17:44:26
 * @Description: 
 */
#include "Timer.h"

std::atomic_int64_t Timer::numCreated_ = 0;

void Timer::restart(Timestamp now)
{
    if (repeat_) {
        expiredTime_ = addTime(now, interval_);
    } else {
        expiredTime_ = Timestamp::invalid();
    }
}