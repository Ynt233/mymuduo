/*
 * @Author: Ynt
 * @Date: 2025-02-07 15:23:12
 * @LastEditTime: 2025-02-07 21:10:20
 * @Description:
 */
#ifndef MYMUDUO_ASYNCLOGGER
#define MYMUDUO_ASYNCLOGGER

#include "CountDownLatch.h"
#include "noncopyable.h"
#include "Thread.h"
#include "Logger.h"

#include <atomic>
#include <vector>

using namespace mymuduo;
namespace mymuduo {

class AsyncLogger : noncopyable
{
public:

    AsyncLogger(const std::string& basename = "/home/yu/muduo/logs/",
                off_t rollSize = 1024 * 1024 * 500,
                int flushInterval = 3);

    ~AsyncLogger()
    {
    if (running_)
    {
        stop();
    }
    }

    void append(const char* logline, int len);

    void start()
    {
        running_ = true;
        thread_.start();
        latch_.wait();
    }

    void stop() 
    {
        running_ = false;
        cond_.notify_one();
        thread_.join();
    }

private:

    void threadFunc();

    using Buffer = FixedBuffer<kLargeBuffer>;
    using BufferVector = std::vector<std::unique_ptr<Buffer>>;
    using BufferPtr = BufferVector::value_type;

    const int flushInterval_;
    std::atomic<bool> running_;
    const std::string basename_;
    const off_t rollSize_;
    Thread thread_;
    CountDownLatch latch_;
    std::mutex mutex_;
    std::condition_variable cond_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
};

}

#endif  // MUDUO_BASE_ASYNCLOGGING_H

