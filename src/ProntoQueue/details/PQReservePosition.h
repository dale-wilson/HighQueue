/// @file PQAllocator.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <ProntoQueue/details/PQDefinitions.h>
#include <Common/Spinlock.h>

namespace MPass
{
	namespace ProntoQueue
	{
		struct PQReservePosition
		{
            AtomicPosition reservePosition_;
            Position reserveSoloPosition_;
            Spinlock reserveSpinlock_;
        };
    }  
}