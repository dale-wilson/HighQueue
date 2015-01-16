/// @file Connection.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <HighQueue/WaitStrategy.h>

namespace HighQueue
{
    /// @brief Information needed to construct an InfiniteVectgor
    struct CreationParameters
    {
        /// @brief How should a producer wait if the queue is full.
        WaitStrategy producerWaitStrategy_;
        /// @brief How should a consumer wait if no data is available.
        WaitStrategy consumerWaitStrategy_;
        /// @brief What happens when the queue is full and no consumer is available?
        bool discardMessagesIfNoConsumer_;
        /// @brief How many entries in the visible window of the HighQueue
        size_t entryCount_;
        /// @brief What is the minimum size of a message (will be rounded up to the next cache line boundary)
        size_t messageSize_;
        /// @brief What is the minimum number of messages needed (one extra may be allocated just because...)
        /// Not needed when an external memory pool will be used.
        size_t messageCount_;

        CreationParameters(
            const WaitStrategy & producerWaitStrategy_,
            const WaitStrategy & consumerWaitStrategy_,
            bool discardMessagesIfNoConsumer,
            size_t entryCount,
            size_t messageSize,
            size_t messageCount = 0
            )
            : producerWaitStrategy_(producerWaitStrategy_)
            , consumerWaitStrategy_(consumerWaitStrategy_)
            , discardMessagesIfNoConsumer_(discardMessagesIfNoConsumer)
            , entryCount_(entryCount)
            , messageSize_(messageSize)
            , messageCount_(messageCount)
        {}
    };
}
