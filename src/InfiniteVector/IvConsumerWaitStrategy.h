/// @file IvConsumerWaitStrategy.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "IvDefinitions.h"

namespace MPass
{
namespace InfiniteVector
{
    /// @brief How should a consumer wait if no data is available?
    /// Each strategy is tried in order until its count is exceeded (or data arrives)
    /// A count of zero means skip that strategy
    /// A count of FOREVER means continue with that strategy -- do not continue to the following strategies.
    ///
    /// TODO: Support not implemented fully yet!  Right now it yields forever.
    /// TODO: ultimate timeout and or the ability to cancel for shut down purposes is not implemented yet!
    /// 
    struct IvConsumerWaitStrategy
    {
        static const size_t FOREVER = size_t(~0u);
        size_t spinCount_;
        size_t yieldCount_;
        size_t sleepCount_;
        std::chrono::nanoseconds sleepPeriod_;
        std::chrono::nanoseconds mutexWaitTimeout_;
        explicit IvConsumerWaitStrategy(
            size_t spinCount = FOREVER,
            size_t yieldCount = FOREVER,
            size_t sleepCount = FOREVER,
            std::chrono::nanoseconds sleepPeriod = std::chrono::nanoseconds(),
            std::chrono::nanoseconds mutexWaitTimeout = std::chrono::nanoseconds())
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