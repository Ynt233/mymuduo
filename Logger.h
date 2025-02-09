/*
 * @Author: Ynt
 * @Date: 2025-02-07 17:58:20
 * @LastEditTime: 2025-02-07 23:26:11
 * @Description: 
 */
#ifndef MYMUDUO_LOGGER
#define MYMUDUO_LOGGER

#include <string>
#include <cstring>
#include <functional>
#include "noncopyable.h"

namespace mymuduo 
{
  const int kSmallBuffer = 4000;
  const int kLargeBuffer = 4000*1000;
}
template<int SIZE>
class FixedBuffer : noncopyable
{
 public:
  FixedBuffer()
    : cur_(data_)
  {
  }

  ~FixedBuffer()
  {
  }

  void append(const char* /*restrict*/ buf, size_t len)
  {
    if (static_cast<size_t>(avail()) > len)
    {
      ::memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  const char* data() const { return data_; }
  int length() const { return static_cast<int>(cur_ - data_); }

  // write to data_ directly
  char* current() { return cur_; }
  int avail() const { return static_cast<int>(end() - cur_); }
  void add(size_t len) { cur_ += len; }

  void reset() { cur_ = data_; }
  void bzero() { ::bzero(data_, sizeof data_); }

  std::string toString() const { return std::string(data_, length()); }

 private:
  const char* end() const { return data_ + sizeof data_; }

  char data_[SIZE];
  char* cur_;
};

enum LogLevel
{
    INFO,
    WARNING,
    ERROR,
    FATAL,
    DEBUG,
};

class Logger : noncopyable
{
public:
    using LogBuffer = FixedBuffer<mymuduo::kSmallBuffer>;

    static Logger& GetInstance();
    void setLogLevel(int level);
    void log(std::string msg);
    // using OutputFunc = void (*)(const char*, int);
    using OutputFunc = std::function<void(const char*, int)>;
    // using FlushFunc = void (*)();
    using FlushFunc = std::function<void()>;
    static void setOutPut(OutputFunc);
    static void setFlush(FlushFunc);

private:
    int logLevel_;
    Logger() {}
};

#define LOG_INFO(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::GetInstance(); \
        logger.setLogLevel(INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0) 

#define LOG_WARNING(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::GetInstance(); \
        logger.setLogLevel(WARNING); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0) 

#define LOG_ERROR(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::GetInstance(); \
        logger.setLogLevel(ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0) 

#define LOG_FATAL(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::GetInstance(); \
        logger.setLogLevel(FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
        exit(-1); \
    } while(0) 

#ifdef MYMUDUO_DEBUG
#define LOG_DEBUG(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::GetInstance(); \
        logger.setLogLevel(DEBUG); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0) 
#else
    #define LOG_DEBUG(logmsgFormat, ...)
#endif

#endif