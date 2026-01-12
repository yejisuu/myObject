#pragma once

#include <functional>
#include <memory>


class Buffer;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

// 连接回调
using ConnectionCallback = std::function<void (const TcpConnectionPtr&)>;

// 收到消息的回调
using RecvMessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*)>;

// 发送完成的回调
using WriteOverCallback = std::function<void(const TcpConnectionPtr&)>;

// 连接关闭回调(内部)
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

