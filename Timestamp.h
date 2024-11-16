/*
 * @Author: Ynt
 * @Date: 2024-11-13 11:08:36
 * @LastEditTime: 2024-11-15 13:58:49
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
    std::string toString() const;
private:
    int64_t microSecondsSinceEpoch_;
};
