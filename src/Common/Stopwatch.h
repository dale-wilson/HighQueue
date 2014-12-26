//@brief Stopwatch.cpp defines a stopwatch for performance measurement
#pragma once


namespace MPass
{

#ifdef _WIN32

    class Stopwatch
    {
    public:
        static const uint64_t microsecondsPerSecond = 1000LL * 1000LL;

        Stopwatch()
        : frequency_(0)
        , start_(0)
        {
            LARGE_INTEGER freq;
            (void)QueryPerformanceFrequency(&freq);
            frequency_ = (uint64_t)freq.QuadPart;
            start_ = now();
        }

        void reset()
        {
            start_ = now();
        }

        uint64_t microseconds() const
        {
            uint64_t lapse = now() - start_;
            return (lapse * microsecondsPerSecond) / frequency_;
        }

        static uint64_t now()
        {
            LARGE_INTEGER time;
            (void)QueryPerformanceCounter(&time);
            return (uint64_t) time.QuadPart;
        }

    private:
        uint64_t frequency_;
        uint64_t start_;
    };

#else _WIN32
#error need non windows version of stopwatch
#endif // _WIN32

} // namespace MPass