/*
 * @Author: Ynt
 * @Date: 2024-11-15 19:10:50
 * @LastEditTime: 2024-11-20 11:19:10
 * @Description: 
 */
#include <unistd.h>
#include <errno.h>
#include <sys/uio.h>
#include "Buffer.h"

const char Buffer::kCRLF[] = "\r\n";

ssize_t Buffer::readFd(int fd, int *saveErrno)
{
    char extraBuf[65536] = {0};
    iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writeIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof(extraBuf);

    const int iovcnt = writable < sizeof(extraBuf) ? 2 : 1;
    ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0) {
        *saveErrno = errno;
    } else if (n <= writable) {
        writeIndex_ += n;
    } else {
        writeIndex_ = buffer_.size();
        append(extraBuf, n - writable);
    }

    return n;
}

ssize_t Buffer::writeFd(int fd, int *saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0) {
        *saveErrno = errno;
    }

    return n;
}