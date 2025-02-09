/*
 * @Author: Ynt
 * @Date: 2025-02-07 19:56:39
 * @LastEditTime: 2025-02-07 20:36:42
 * @Description: 
 */

// #include <mymuduo/Logger.h>
#include <mymuduo/AsyncLogger.h>
#include <mymuduo/Timestamp.h>

#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

using namespace mymuduo;
off_t kRollSize = 500*1000*1000;

AsyncLogger* g_asyncLog = NULL;

void asyncOutput(const char* msg, int len)
{
  g_asyncLog->append(msg, len);
}

void bench()
{
  // mymuduo::Logger::setOutput(asyncOutput);
  Logger::setOutPut(asyncOutput);

  int cnt = 0;
  const int kBatch = 1000;
  std::string empty = " ";
  // std::string longStr(3000, 'X');
  std::string longStr(300, 'X');
  longStr += " ";

  for (int t = 0; t < 30; ++t)
  {
    Timestamp start = Timestamp::now();
    for (int i = 0; i < kBatch; ++i)
    {
      // LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
      //          << (longLog ? longStr : empty)
      //          << cnt;
      LOG_INFO("Hello 0123456789 abcdefghijklmnopqrstuvwxyz %s%d", longStr, cnt);
      // LOG_INFO("test\n");
      ++cnt;
    }
    Timestamp end = Timestamp::now();
    printf("%f\n", timeDifference(end, start)*1000000/kBatch);
    struct timespec ts = { 0, 500*1000*1000 };
    nanosleep(&ts, NULL);
  }
}

int main()
{
  {
    // set max virtual memory to 2GB.
    size_t kOneGB = 1000*1024*1024;
    rlimit rl = { 2*kOneGB, 2*kOneGB };
    setrlimit(RLIMIT_AS, &rl);
  }

  printf("pid = %d\n", getpid());

  AsyncLogger log;
  log.start();
  g_asyncLog = &log;

  bench();
}
