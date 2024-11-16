/*
 * @Author: Ynt
 * @Date: 2024-11-15 12:06:53
 * @LastEditTime: 2024-11-15 15:52:35
 * @Description: 
 */
#include <semaphore.h>
#include "CurrentThread.h"
#include "Thread.h"

std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string &name)
    : func_(std::move(func)),
    started_(false),
    joined_(false),
    tid_(0),
    name_(name)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_) {
        thread_->detach(); // set detached thread
    }
}

void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        tid_ = CurrentThread::tid();

        sem_post(&sem);

        // start a new thread to execute the corresponding thread function
        func_(); 
    }));

    // must wait for the tid_ value of the newly created thread
    sem_wait(&sem);
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if (name_.empty()) {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}