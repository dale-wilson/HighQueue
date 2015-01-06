/// @file HSQAllocator.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <HSQueue/details/HSQDefinitions.h>
#include <Common/Spinlock.h>

namespace HSQueue
{
	struct HSQReservePosition
	{
        AtomicPosition reservePosition_;
        Position reserveSoloPosition_;
        Spinlock reserveSpinlock_;
    };
}  
