/*
 * @Author: Ynt
 * @Date: 2024-11-13 11:08:36
 * @LastEditTime: 2025-02-09 16:14:07
 * @Description: 
 */
#pragma once
#include <iostream>
#include <string>

class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    static Timestamp now();
    void swap(Timestamp& that) { std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_); }
    std::string toFormattedString(bool showMicroseconds = true) const;
    std::string toString() const;
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    static Timestamp invalid() { return Timestamp(); }

    static const int kMicroSecondsPerSecond = 1000 * 1000;
private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator < (const Timestamp& t1, const Timestamp& t2)
{
    return t1.microSecondsSinceEpoch() < t2.microSecondsSinceEpoch();
}

inline double timeDifference(Timestamp high, Timestamp low)
{
  int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}