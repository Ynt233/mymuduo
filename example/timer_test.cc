/*
 * @Author: Ynt
 * @Date: 2025-02-09 16:45:28
 * @LastEditTime: 2025-02-09 18:35:33
 * @Description: 
 */

#include <mymuduo/EventLoop.h>
#include <mymuduo/EventLoopThread.h>
#include <mymuduo/Thread.h>

#include <stdio.h>
#include <unistd.h>

int cnt = 0;
EventLoop* g_loop;

void printTid()
{
  printf("pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
  printf("now %s\n", Timestamp::now().toFormattedString().c_str());
}

void print(const char* msg)
{
  printf("msg %s %s\n", Timestamp::now().toFormattedString().c_str(), msg);
  if (++cnt == 10)
  {
    g_loop->quit();
  }
}

void cancel(TimerId timer)
{
  g_loop->cancelTimer(timer);
  printf("cancelled at %s\n", Timestamp::now().toFormattedString().c_str());
}

int main()
{
  printTid();
  sleep(1);
  {
    EventLoop loop;
    g_loop = &loop;

    print("main");
    loop.runAfter(1, std::bind(print, "test1"));
    loop.runAfter(1.5, std::bind(print, "test1.5"));
    loop.runAfter(2.5, std::bind(print, "test2.5"));
    loop.runAfter(3.5, std::bind(print, "test3.5"));
    TimerId t45 = loop.runAfter(4.5, std::bind(print, "test4.5"));
    loop.runAfter(4.2, std::bind(cancel, t45));
    loop.runAfter(4.8, std::bind(cancel, t45));
    TimerId t3 = loop.runEvery(3, std::bind(print, "every3"));
    loop.runAfter(20.001, std::bind(cancel, t3));

    loop.loop();
    print("main loop exits");
  }
  sleep(1);
  {
    EventLoopThread loopThread;
    EventLoop* loop = loopThread.startLoop();
    loop->runAfter(2, printTid);
    sleep(3);
    print("thread loop exits");
  }
}