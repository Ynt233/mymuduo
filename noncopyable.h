/*
 * @Author: Ynt
 * @Date: 2024-11-13 10:09:14
 * @LastEditTime: 2024-11-13 11:07:59
 * @Description: noncopyable class base on C++11 standard
 */
#pragma once
/*
After noncopyable is inherited, derived objects can be constructed and destructed normally, but cannot be copied and assigned
*/
class noncopyable 
{
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;
protected:
    noncopyable()=default;
    ~noncopyable()=default;
};