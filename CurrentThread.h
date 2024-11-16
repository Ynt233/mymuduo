/*
 * @Author: Ynt
 * @Date: 2024-11-14 14:50:01
 * @LastEditTime: 2024-11-14 14:58:02
 * @Description: 
 */
#pragma once
#include <sys/syscall.h>
#include <unistd.h>

namespace CurrentThread
{
    extern __thread int t_cachedTid;

    void cacheTid();

    inline int tid()
    {
        if (__builtin_expect(t_cachedTid == 0, 0)) {
            cacheTid();
        }
        return t_cachedTid;
    }
}