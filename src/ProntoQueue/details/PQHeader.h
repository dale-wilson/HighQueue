/// @file PQHeader.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <ProntoQueue/details/PQDefinitions.h>
//ProntoQueue/PQDefinitions.h>
#include <ProntoQueue/CreationParameters.h>
#include <ProntoQueue/details/PQAllocator.h>

namespace MPass
{
namespace ProntoQueue
{
    struct PQHeader
    {
        Signature signature_;
        uint8_t version_;
        char name_[32];
        size_t entryCount_;
        Offset entries_;
        Offset readPosition_;
        Offset publishPosition_;
        Offset reservePosition_;
        Offset memoryPool_;

        ConsumerWaitStrategy consumerWaitStrategy_;
        std::mutex consumerWaitMutex_;
        std::condition_variable consumerWaitConditionVariable_;

	    PQHeader(
            const std::string & name, 
            PQAllocator & allocator, 
            const CreationParameters & parameters);
        void allocateInternalMessages();
        void releaseInternalMessages();
    };
}
}