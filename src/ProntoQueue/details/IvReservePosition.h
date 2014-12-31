/// @file IvAllocator.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <ProntoQueue/details/IvDefinitions.h>
#include <Common/Spinlock.h>

namespace MPass
{
	namespace ProntoQueue
	{
		struct IvReservePosition
		{
            AtomicPosition reservePosition_;
            Position reserveSoloPosition_;
            Spinlock reserveSpinlock_;
        };
    }  
}