/*
 * @Author: Ynt
 * @Date: 2024-11-15 15:41:40
 * @LastEditTime: 2024-11-16 10:58:46
 * @Description: 
 */
#include <errno.h>
#include <string>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"
#include "TcpConnection.h"

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr) {
        LOG_ERROR("%s:%s:%d main loop is null!\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, const std::string& name, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr)
    : loop_(CheckLoopNotNull(loop)),
    name_(name),
    state_(kConnecting),
    reading_(true),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    highWaterMark_(64 * 1024 * 1024) // 64M
{
    channel_->setReadCallback(std::bind(&TcpConnection::readHandler, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::writeHandler, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::closeHandler, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::errorHandler, this));

    LOG_DEBUG("TcpConnection %s at fd= %d\n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG("TcpConnection %s at fd= %d state is: %d\n", name_.c_str(), sockfd, state_);
}

void TcpConnection::readHandler(Timestamp receiveTime)
{
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) {
        closeHandler();
    } else {
        errno = saveErrno;
        LOG_ERROR("TcpConnection::readHandler failed: %d\n", errno);
        errorHandler();
    }
}

void TcpConnection::writeHandler()
{
    if (channel_->isWriting()) {
        int saveErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) { // write is done
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
            }

            if (state_ == kDisconnecting) {
                shutdownInLoop();
            }
        } else {
            errno = saveErrno;
            LOG_ERROR("TcpConnection::writeHandler error: %d\n", errno);
            errorHandler();
        }
    } else {
        LOG_WARNING("TcpConnection fd= %d is unwritable \n", channel_->fd());
    }
}

void TcpConnection::closeHandler()
{
    LOG_DEBUG("fd= %d, state= %d\n", channel_->fd(), state_);
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    if (connectionCallback_)
    {
        connectionCallback_(connPtr);
    }
    
    if (closeCallback_)
    {
        closeCallback_(connPtr);
    }
}

void TcpConnection::errorHandler()
{
    int opt;
    int err;
    socklen_t optlen = sizeof opt;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &opt, &optlen) < 0) {
        err = errno;
    } else {
        err = opt;
    }
    LOG_ERROR("TcpConnection::errorHandler name%s - SO_ERROR:%d\n", name_.c_str(), err);
}

void TcpConnection::send(const void* message, int len)
{
    send(std::string(static_cast<const char*>(message)));
}

void TcpConnection::send(const std::string& message)
{
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message.c_str(), message.size());
        } else {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message.c_str(), message.size()));
        }
    }
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading(); // register channel's epollin event to poller

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::sendInLoop(const void* message, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if (state_ == kDisconnected) {
        LOG_ERROR("TcpConnection is disconnected\n");
        return;
    }

    // means the first time for channel to send msg, and buffer has no msg ready to be sent
    // or previous msgs are all sended
    if (channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), message, len);
        if (nwrote >= 0) {
            remaining = len - nwrote; 
            if (remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR("TcpConnection::sendInLoop\n");
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    if (!faultError && remaining > 0) {
        // the rest of data ready to be sent
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_) {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char*>(message) + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdownInLoop()
{
    if (!channel_->isWriting()) { // all data in outputBuffer is sent
        socket_->shutdownWrite(); // EPOLLHUP
    }
}