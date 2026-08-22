#pragma once
#include <cstdint>
#include <string>
#include <ctime>
namespace muduo
{
class Timestamp
{
public:
    Timestamp()
        : microSecondsSinceEpoch_(0)
    {}
    explicit Timestamp(int64_t microSecondsSinceEpoch)
        : microSecondsSinceEpoch_(microSecondsSinceEpoch)
    {}
    static Timestamp now()
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME,&ts);
        return Timestamp(ts.tv_sec * 1000000LL + ts.tv_nsec / 1000);
    }
    int64_t microSecondsSinceEpoch() const
    {
        return microSecondsSinceEpoch_;
    }
    double secondsSinceEpoch() const
    {
        return microSecondsSinceEpoch_ / 1000000.0;
    }
    bool valid() const { return microSecondsSinceEpoch_ > 0; }
    Timestamp addSeconds(double seconds) const
    {
        return Timestamp(microSecondsSinceEpoch_ + static_cast<int64_t>(seconds * 1000000));
    }
    bool operator<(const Timestamp& rhs) const
    {
        return microSecondsSinceEpoch_ < rhs.microSecondsSinceEpoch_;
    }
    bool operator==(const Timestamp& rhs) const
    {
        return microSecondsSinceEpoch_ == rhs.microSecondsSinceEpoch_;
    }
    std::string toString() const
    {
        return std::to_string(microSecondsSinceEpoch_);
    }
    std::string toFormattedString() const
    {
        time_t seconds = microSecondsSinceEpoch_ / 1000000;
        int microsecs = microSecondsSinceEpoch_ % 1000000;

        struct tm tm_time;
        localtime_r(&seconds,&tm_time);

        char buf[32];
        snprintf(buf,sizeof(buf),
                 "%04d%02d%02d %02d:%02d:%02d.%06d",
                tm_time.tm_year + 1900,
                tm_time.tm_mon + 1,
                tm_time.tm_mday,
                tm_time.tm_hour,
                tm_time.tm_min,
                tm_time.tm_sec,
                microsecs);
        return std::string(buf);
    }
private:
    int64_t microSecondsSinceEpoch_;
};
}
