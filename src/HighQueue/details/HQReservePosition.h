/// @file HQAllocator.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <HighQueue/details/HQDefinitions.h>
#include <Common/Spinlock.h>

namespace HighQueue
{
	struct HighQReservePosition
	{
        AtomicPosition reservePosition_;
        Spinlock reserveSpinlock_;
    };
}  
