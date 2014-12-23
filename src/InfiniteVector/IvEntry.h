/// @file IvEntry.h
#pragma once
#include "IvDefinitions.h"
#include <InfiniteVector/IvAllocator.h>
#include <Buffers/Buffer.h>
namespace MPass
{
	namespace InfiniteVector
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
            Buffers::Buffer buffer_;
            Status status_;

            IvEntry()
                : status_(Status::EMPTY)
            {
            }

            static size_t alignedSize()
            {
                return IvAllocator::align(sizeof(IvEntry), CacheLineSize);
            }

		} POST_CACHE_ALIGN;
	}
}