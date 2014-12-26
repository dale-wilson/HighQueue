/// @file IvHeader.h
#pragma once
#include "IvDefinitions.h"
#include <InfiniteVector/IvCreationParameters.h>
#include <InfiniteVector/IvAllocator_fwd.h>
#include <Buffers/MemoryBlockAllocator.h>

namespace MPass
{
namespace InfiniteVector
{
    struct IvHeader
    {
        Signature signature_;
        uint8_t version_;
        char name_[32 - sizeof(uint8_t) - sizeof(Signature)];
        size_t entryCount_;
        Offset entries_;
        Offset readPosition_;
        Offset publishPosition_;
        Offset reservePosition_;

        Buffers::MemoryBlockInfo blockInfo_;
        bool consumerWaitsViaMutexCondition_;
        std::mutex consumerWaitMutex_;
        std::condition_variable consumerWaitConditionVariable_;

	    IvHeader(
            const std::string & name, 
            IvAllocator & allocator, 
            const IvCreationParameters & parameters);
    };
}
}