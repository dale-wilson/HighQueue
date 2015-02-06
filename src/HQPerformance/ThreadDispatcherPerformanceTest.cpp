#include "Common/HighQueuePch.h"

#ifdef _WIN32 /// right now this test only works on windows.
#define BOOST_TEST_NO_MAIN HighQueuePerformanceTest
#include <boost/test/unit_test.hpp>

/// This is not a test of HighQueue.  It is a test of the operating system's dispatching algorithm.
/// The Windows dispatcher seems to be brain-dead.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <Common/Stopwatch.h>
#include <iostream>
#include <cstdint>
#include <limits>
#include <thread>
#include <atomic>
#include <vector>

#if 0
#   define YIELD() std::this_thread::yield()
#   define TECHNIQUE "std::this_thread::yield()"
#elif 0
#   define YIELD() ::SwitchToThread()
#   define TECHNIQUE "::SwitchToThread()"
#elif 0
#   define YIELD() ::Sleep(0)
#   define TECHNIQUE "::Sleep(0)"
#elif 0
#   define YIELD() std::this_thread::sleep_for(std::chrono::milliseconds(1))
#   define TECHNIQUE "std::this_thread::sleep_for(std::chrono::milliseconds(1))"
#elif 0
#   define YIELD() ::Sleep(1)
#   define TECHNIQUE "::Sleep(1)"
#elif 01
#   define YIELD() ::YieldProcessor()
#   define TECHNIQUE "::YieldProcessor()"
#else 
#define YIELD() do{\
    size_t initial=position; \
    while(initial == position)\
    {\
        /*spin*/;\
    }\
    } while(false)
#define TECHNIQUE "SpinUntilPositionChanges"
#endif

namespace {
    const size_t entryCount = 1000000;
    uint8_t threadIds[entryCount];
    std::atomic<bool> go(false);
    std::atomic<size_t> threadsRunning(0);
    std::atomic<size_t> position(0);
    const uint64_t microsecondsPerSecond = 1000000;

    void threadFunction(uint8_t threadId)
    {
        ++threadsRunning;
        while (!go)
        {
            std::this_thread::yield();
        }
        while (true)
        {
            auto pos = position++;
            if (pos >= entryCount)
            {
                return;
            }
            threadIds[pos] = threadId;
            YIELD();
        }
    }
}

BOOST_AUTO_TEST_CASE(testOSThreadDispatcher)
{
    ///////////////
    // Capture data
    auto coreCount = std::thread::hardware_concurrency();
    uint32_t overbook = -2;
    uint8_t threadCount = uint8_t(coreCount + overbook);

    LARGE_INTEGER frequency;
    ::QueryPerformanceFrequency(&frequency);
    std::vector<std::thread> threads;
    for (uint8_t nth = 0; nth < threadCount; ++nth)
    {
        threads.emplace_back(std::bind(threadFunction, nth));
    }
    while (threadsRunning < threadCount)
    {
        std::this_thread::yield();
    }

    LARGE_INTEGER start;
    ::QueryPerformanceCounter(&start);
    go = true;
    for (auto & thread : threads)
    {
        thread.join();
    }
    LARGE_INTEGER stop;
    ::QueryPerformanceCounter(&stop);

    ///////////////
    // Analyze data
    std::vector<size_t> previous(threadCount, size_t(0));
    std::vector<size_t> minGaps(threadCount, std::numeric_limits<size_t>::max());
    std::vector<size_t> maxGaps(threadCount, 0);
    size_t overallMaxGap = 0;
    size_t startMaxGap = 0;
    size_t endMaxGap = 0;
    uint8_t threadIdMaxGap = 0;

    struct Bucket
    {
        size_t limit_;
        size_t count_;
        Bucket(size_t limit)
            : limit_(limit)
            , count_(0)
        {}
    };

    std::vector<Bucket> buckets;
    for (size_t limit = 1; limit < 10; ++limit)
    {
        buckets.emplace_back(limit);
    }
    for (size_t limit = 10; limit < 100; limit += 10)
    {
        buckets.emplace_back(limit);
    }
    for (size_t limit = 100; limit < 1000; limit += 100)
    {
        buckets.emplace_back(limit);
    }
    for (size_t limit = 1000; limit < 10000; limit += 1000)
    {
        buckets.emplace_back(limit);
    }
    buckets.emplace_back(std::numeric_limits<size_t>::max());

    for (size_t pos = 0; pos < entryCount; ++pos)
    {
        auto threadId = threadIds[pos];
        auto gap = pos - previous[threadId];
        if (gap < minGaps[threadId])
        {
            minGaps[threadId] = gap;
        }
        if (gap > maxGaps[threadId])
        {
            maxGaps[threadId] = gap;
        }
        if (gap > overallMaxGap)
        {
            overallMaxGap = gap;
            startMaxGap = previous[threadId];
            endMaxGap = pos;
            threadIdMaxGap = threadId;
        }

        for (auto & bucket : buckets)
        {
            if (gap < bucket.limit_)
            {
                ++bucket.count_;
                break;
            }
        }

        previous[threadId] = pos;

    }

    ///////////////////
    // Display results
    std::cout << "--------------------" << std::endl;
    std::cout << TECHNIQUE << std::endl;
    std::cout << uint16_t(threadCount) << " threads on " << coreCount << " cores." << std::endl;
    std::cout << "--------------------" << std::endl;
    for (size_t pos = 0; pos < threadCount; ++pos)
    {
        std::cout << pos << ": min = " << minGaps[pos] << " max = " << maxGaps[pos] << std::endl;
    }

    std::cout << "Overall max gap: [" << startMaxGap << ", " << endMaxGap << "] = " << overallMaxGap << " for thread " << uint16_t(threadIdMaxGap) << std::endl;
    auto microseconds = (stop.QuadPart - start.QuadPart) * microsecondsPerSecond / frequency.QuadPart;
    std::cout << microseconds << " uSec." << std::endl;

    size_t previousLimit = 0;
    for (auto & bucket : buckets)
    {
        if (bucket.count_ != 0)
        {
            std::cout << '[' << std::setw(4) << previousLimit << '-' << std::setw(4) << bucket.limit_ << ") : " << bucket.count_ << std::endl;
        }
        previousLimit = bucket.limit_;
    }

    std::cout << "===================================================================" << std::endl;

    BOOST_WARN_LT(overallMaxGap, 3*threadCount);
    BOOST_CHECK_LT(overallMaxGap, 3*threadCount);
}

#endif // _WIN32