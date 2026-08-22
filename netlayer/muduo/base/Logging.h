#pragma once
#include <iostream>

namespace muduo
{
    class Logger
    {
    public:
        explicit Logger(std::ostream& os) : _os(os) {}
        template <typename T>
        Logger& operator<<(const T& v)
        {
            _os << v;
            return *this;
        }
        ~Logger() { _os << std::endl; }
    private:
        std::ostream& _os;
};
#define LOG_INFO  muduo::Logger(std::cout) << "[INFO] "
#define LOG_ERROR muduo::Logger(std::cerr) << "[ERROR] "
#define LOG_WARN  muduo::Logger(std::cerr) << "[WARN] "
#define LOG_DEBUG muduo::Logger(std::cerr) << "[DEBUG] "

}
