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
        #ifdef _WIN32
        // Windows thread dispatcher is defective.
        // As far as I can tell yield (or as Windows says it ::Sleep(0) dispatches to another thread if one is available
        // then puts the yielding thread at the FRONT of the dispatch queue.   If you have two threads yielding because they
        // have nothing to do, it bounces back and forth between them until one of them uses up its time slice, and gets
        // relegated to the end of the queue.   The net efeect of this is serious starvation for threads further back in the queue.
        //
        // The ThreadDispatcherPerformanceTest mesasures this effect.  I have seen 5000 dispatches to yielding threads before
        // a thread with useful work gets dispatched to.
        //
        // This CriticalSection is a work-around for this defect.
        CRITICAL_SECTION reserveCriticalSection_;
        HighQReservePosition()
        {
            ::InitializeCriticalSection(&reserveCriticalSection_);
        }
#else
        Spinlock reserveSpinlock_;
#endif // _WIN32
      
    };
}  
