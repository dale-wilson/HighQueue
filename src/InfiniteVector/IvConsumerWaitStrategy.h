/// @file IvConsumerWaitStrategy.h
#pragma once
#include "IvDefinitions.h"

namespace MPass
{
namespace InfiniteVector
{
    struct IvConsumerWaitStrategy
    {
        static const size_t FOREVER = size_t(~0u);
        size_t spinCount_;
        size_t yieldCount_;
        size_t sleepCount_;
        boost::chrono::nanoseconds sleepPeriod_;
        boost::chrono::nanoseconds mutexWaitTimeout_;
        explicit IvConsumerWaitStrategy(
            size_t spinCount = FOREVER,
            size_t yieldCount = FOREVER,
            size_t sleepCount = FOREVER,
            boost::chrono::nanoseconds sleepPeriod = boost::chrono::nanoseconds(),
            boost::chrono::nanoseconds mutexWaitTimeout = boost::chrono::nanoseconds())
        : spinCount_(spinCount)
        , yieldCount_(yieldCount)
        , sleepCount_(sleepCount)
        , sleepPeriod_(sleepPeriod)
        , mutexWaitTimeout_(mutexWaitTimeout)
        {
        }
    };
}
}