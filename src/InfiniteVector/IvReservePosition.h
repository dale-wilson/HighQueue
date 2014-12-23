/// @file IvAllocator.h
#pragma once
#include <InfiniteVector/IvDefinitions.h>
#include <Common/Spinlock.h>

namespace MPass
{
	namespace InfiniteVector
	{
		struct IvReservePosition
		{
            AtomicPosition reservePosition_;
            Spinlock spinlock_;
        };
    }  
}