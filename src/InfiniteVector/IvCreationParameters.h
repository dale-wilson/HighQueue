/// @file IvConnection.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <InfiniteVector/IvConsumerWaitStrategy.h>

namespace MPass
{
	namespace InfiniteVector
	{
        /// @brief Information needed to construct an InfiniteVectgor
        struct IvCreationParameters
        {
            /// @brief How should a consumer wait if no data is available.
            IvConsumerWaitStrategy strategy_;
            /// @brief How many entries in the visible window of the InfiniteVector
            size_t entryCount_;
            /// @brief What is the minimum size of a buffer (will be rounded up to the next cache line boundary)
            size_t bufferSize_;
            /// @brief What is the minimum number of buffers needed (one extra may be allocated just because...)
            size_t bufferCount_;

            IvCreationParameters(
                const IvConsumerWaitStrategy & strategy_,
                size_t entryCount,
                size_t bufferSize,
                size_t bufferCount
                )
                : strategy_(strategy_)
                , entryCount_(entryCount)
                , bufferSize_(bufferSize)
                , bufferCount_(bufferCount)
            {
            }
        };
	}
}