// @brief Stopwatch.cpp defines a stopwatch for performance measurement

#pragma once
namespace MPass
{
    class Stopwatch
    {
    public:
        static const uint64_t millisecondsPerSecond = 1000LL;
        static const uint64_t microsecondsPerSecond = 1000LL * 1000LL;
        static const uint64_t nanosecondsPerSecond = 1000LL * 1000LL * 1000LL;

        Stopwatch()
        {
            start_ = now();
        }

        void reset()
        {
            start_ = now();
        }

        uint64_t nanoseconds() const
        {
            return now() - start_;
        }

        uint64_t microseconds() const
        {
            return (nanoseconds() * microsecondsPerSecond) / nanosecondsPerSecond;
        }

        uint64_t milliseconds() const
        {
            return (nanoseconds() * millisecondsPerSecond) / nanosecondsPerSecond;
        }

        double seconds() const
        {
            return double(nanoseconds()) / double(nanosecondsPerSecond);
        }

        static uint64_t now()
        {
            auto point = std::chrono::high_resolution_clock::now();
            std::chrono::nanoseconds duration = point.time_since_epoch();
            return duration.count();
        }

    private:
        uint64_t start_;
    };

} // namespace MPass