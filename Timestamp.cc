/*
 * @Author: Ynt
 * @Date: 2024-11-13 11:08:46
 * @LastEditTime: 2025-02-09 18:05:36
 * @Description: Implementations of Timestamp class
 */
#include <sys/time.h>
#include "Timestamp.h"

Timestamp::Timestamp():microSecondsSinceEpoch_(0) {}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch):microSecondsSinceEpoch_(microSecondsSinceEpoch) {}

Timestamp Timestamp::now() 
{
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  int64_t seconds = tv.tv_sec;
  return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

std::string Timestamp::toString() const 
{
  char buf[128] = {0};
  tm *tm_time = localtime(&microSecondsSinceEpoch_);
  snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",  tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday, 
                                                      tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
  return buf;
}

std::string Timestamp::toFormattedString(bool showMicroseconds) const
{
  char buf[64] = {0};
  time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);

  if (showMicroseconds)
  {
    int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microseconds);
  }
  else
  {
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  }
  return buf;
}