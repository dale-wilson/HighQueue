/// @file PQEntry.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <ProntoQueue/details/PQDefinitions.h>
#include <ProntoQueue/details/PQAllocator.h>
#include <ProntoQueue/Message.h>

namespace ProntoQueue
{
	PRE_CACHE_ALIGN
	struct PQEntry
	{
        enum Status : uint8_t
        {
            OK,
            EMPTY,
            SKIP
        };
        ProntoQueue::Message message_;
        Status status_;

        PQEntry()
            : status_(Status::EMPTY)
        {
        }

        static size_t alignedSize()
        {
            return PQAllocator::align(sizeof(PQEntry), CacheLineSize);
        }

	} POST_CACHE_ALIGN;
}
