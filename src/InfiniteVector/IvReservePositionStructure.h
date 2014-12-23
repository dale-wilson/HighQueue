/// @file IvAllocator.h
#pragma once
#include <InfiniteVector/IvDefinitions.h>
#include <Common/Spinlock.h>

namespace MPass
{
	namespace InfiniteVector
	{
		struct IvReservePositionStructure
		{
            AtomicPosition reservePosition_;
            SpinLock spinLock_;
        };
    }  
}