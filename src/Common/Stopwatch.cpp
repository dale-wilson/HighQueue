// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Common/HighQueuePch.h>
#include "Stopwatch.h"

using namespace HighQueue;
#ifdef _WIN32 // use QPC on windows 'cause chrono is brain-dead
namespace{
    uint64_t getQPCFrequency()
    {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        return uint64_t(frequency.QuadPart);
    }

    uint64_t qpcFrequency = getQPCFrequency();
    
}
uint64_t Stopwatch::now()
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return uint64_t(time.QuadPart) * nanosecondsPerSecond / qpcFrequency;
}
#endif // _WIN32
