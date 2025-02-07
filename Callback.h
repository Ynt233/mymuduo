/*
 * @Author: Ynt
 * @Date: 2024-11-15 15:42:28
 * @LastEditTime: 2024-11-15 19:45:02
 * @Description: 
 */
#pragma once
#include <memory>
#include <functional>

class Buffer;
class TcpConnection;
class Timestamp;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;

using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;

using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;
