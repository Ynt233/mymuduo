/*
 * @Author: Ynt
 * @Date: 2024-11-14 12:50:03
 * @LastEditTime: 2024-11-14 15:44:56
 * @Description: 
 */
#include <stdlib.h>
#include "EPollPoller.h"
#include "Poller.h"

Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if (::getenv("MUDUO_USE_POLL")) {
        return nullptr;
    } else {
        return new EPollPoller(loop);
    }
}