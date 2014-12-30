/// @file Connection.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <InfiniteVector/ConsumerWaitStrategy.h>

namespace MPass
{
	namespace InfiniteVector
	{
        /// @brief Information needed to construct an InfiniteVectgor
        struct CreationParameters
        {
            /// @brief How should a consumer wait if no data is available.
            ConsumerWaitStrategy strategy_;
            /// @brief How many entries in the visible window of the InfiniteVector
            size_t entryCount_;
            /// @brief What is the minimum size of a message (will be rounded up to the next cache line boundary)
            size_t messageSize_;
            /// @brief What is the minimum number of messages needed (one extra may be allocated just because...)
            size_t messageCount_;

            CreationParameters(
                const ConsumerWaitStrategy & strategy_,
                size_t entryCount,
                size_t messageSize,
                size_t messageCount
                )
                : strategy_(strategy_)
                , entryCount_(entryCount)
                , messageSize_(messageSize)
                , messageCount_(messageCount)
            {
            }
        };
	}
}