#pragma once
#include <functional>
#include <memory>
#include <muduo/base/Timestamp.h>

namespace muduo
{
namespace net
{

class TcpConnection;
class Buffer;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using TimerCallback = std::function<void()>;

}
}
