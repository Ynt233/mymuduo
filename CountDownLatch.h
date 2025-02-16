/*
 * @Author: Ynt
 * @Date: 2025-02-07 17:31:40
 * @LastEditTime: 2025-02-07 17:32:43
 * @Description: 
 */

#ifndef MYMUDUO_COUNTDOWNLATCH
#define MYMUDUO_COUNTDOWNLATCH

#include <mutex>
#include <condition_variable>

#include "noncopyable.h"

namespace mymuduo
{

class CountDownLatch : public noncopyable
{
public:
    explicit CountDownLatch(int count);  //倒数几次
    void wait();
    void countDown();

    int getCount();

private:
    std::mutex mutex_;
    std::condition_variable condition_;
    int count_;
};

}

#endif