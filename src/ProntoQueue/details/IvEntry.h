/// @file IvEntry.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <ProntoQueue/details/PQDefinitions.h>
#include <ProntoQueue/details/PQAllocator.h>
#include <ProntoQueue/Message.h>

namespace MPass
{
	namespace ProntoQueue
	{
		PRE_CACHE_ALIGN
		struct IvEntry
		{
            enum Status : uint8_t
            {
                OK,
                EMPTY,
                SKIP
            };
            ProntoQueue::Message message_;
            Status status_;

            IvEntry()
                : status_(Status::EMPTY)
            {
            }

            static size_t alignedSize()
            {
                return PQAllocator::align(sizeof(IvEntry), CacheLineSize);
            }

		} POST_CACHE_ALIGN;
	}
}