/*
 * @Author: Ynt
 * @Date: 2024-11-13 14:21:32
 * @LastEditTime: 2024-11-16 16:12:50
 * @Description: encapsulates sockfd and its binding events, for example EPOLLIN
 * binds the event that poller returns
 */

#pragma once
#include <functional>
#include <memory>
#include "Logger.h"
#include "noncopyable.h"
#include "Timestamp.h"

class EventLoop;

class Channel : public noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int sockfd);
    ~Channel();

    void eventHandler(Timestamp receiveTime);

    void setReadCallback(ReadEventCallback cb) { readEventCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // Prevent channel from being removed 
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int recv) { revents_ = recv; }

    // set the state of corresponding event of fd
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }

    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isReading() const { return events_ & kReadEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }

    int state() { return state_; }
    void set_state(int state) { state_ = state; }

    EventLoop *ownerLoop() { return loop_; }
    void remove();

private:

    void update();
    void eventHandlerWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_;
    const int fd_; // fd object listened by poller
    int events_; // events binding to fd_
    int revents_; // received events returned by poller
    int state_;

    std::weak_ptr<void> tie_; // to supervise if the channel is removed
    bool tied_;
    bool eventHandling_;
    bool addedToLoop_;

    // Channel can perceive the revents, so it is used to call the corresponding callback function 
    ReadEventCallback readEventCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};