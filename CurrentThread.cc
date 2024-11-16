/*
 * @Author: Ynt
 * @Date: 2024-11-14 14:50:08
 * @LastEditTime: 2024-11-14 14:57:11
 * @Description: 
 */
#include "CurrentThread.h"

namespace CurrentThread
{
    __thread int t_cachedTid = 0;

    void cacheTid()
    {
        if (t_cachedTid == 0) {
            // Using Linux syscall to get current thread id
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
}