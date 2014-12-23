/// @file IvConnection.h
#pragma once

#include <InfiniteVector/IvConsumerWaitStrategy.h>

namespace MPass
{
	namespace InfiniteVector
	{
        struct IvCreationParameters
        {
            IvConsumerWaitStrategy strategy_;
            size_t entryCount_;
            size_t bufferSize_;
            size_t bufferCount_;

            IvCreationParameters(
                const IvConsumerWaitStrategy & strategy_,
                size_t entryCount,
                size_t bufferSize = 0u,
                size_t bufferCount = 0u
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