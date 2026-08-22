#pragma once

namespace muduo
{

class noncopyable
{
protected:
    noncopyable() = default;
    ~noncopyable() = default;

    noncopyable(const noncopyable&) = delete;
    noncopyable& operator = (noncopyable&) = delete;
};

}
