// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

namespace HighQueue
{
    /// @brief Stopwatch measures time intervals
    /// It begins running when it is constructed.
    /// Lapse times can be captured at any time in the desired uint by calling
    /// one of the *seconds() methods.
    /// The watch continues to run after a lapsed time is captured.
    ///
    /// The reset() method resets the lapse time to zero.  The watch continues to run.
    class Stopwatch
    {
    public:
        /// @brief Convenient number for time calculaions.
        static const uint64_t millisecondsPerSecond = 1000LL;
        /// @brief Convenient number for time calculaions.
        static const uint64_t microsecondsPerSecond = 1000LL * 1000LL;
        /// @brief Convenient number for time calculaions.
        static const uint64_t nanosecondsPerSecond = 1000LL * 1000LL * 1000LL;

        /// @brief Construct the watch, start the clock
        Stopwatch()
         : start_(now())
        {
        }

        /// @brief Reset the lapsed time to zero; continue running.
        void reset()
        {
            start_ = now();
        }

        /// @brief Capture the current lapsed time in nanoseconds.
        uint64_t nanoseconds() const
        {
            return now() - start_;
        }

        /// @brief Capture the current lapsed time in microseconds.
        uint64_t microseconds() const
        {
            return (nanoseconds() * microsecondsPerSecond) / nanosecondsPerSecond;
        }

        /// @brief Capture the current lapsed time in milliseconds.
        uint64_t milliseconds() const
        {
            return (nanoseconds() * millisecondsPerSecond) / nanosecondsPerSecond;
        }

        /// @brief Capture the current lapsed time in seconds.
        double seconds() const
        {
            return double(nanoseconds()) / double(nanosecondsPerSecond);
        }

        /// @brief Get the current time in nanoseconds in some arbitrary epoch.
        static uint64_t now()
        {
            auto point = std::chrono::high_resolution_clock::now();
            std::chrono::nanoseconds duration = point.time_since_epoch();
            return duration.count();
        }

    private:
        uint64_t start_;
    };

} // namespace HighQueue
