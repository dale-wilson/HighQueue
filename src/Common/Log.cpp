// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Common/HighQueuePch.h>
#include "Log.h"

using namespace HighQueue;
#ifdef _DEBUG
Log::Level Log::runtimeLevel = Log::DEBUG;
#else
Log::Level Log::runtimeLevel = Log::INFO;
#endif

void Log::log(Log::Level level, const char * file, const char * function, uint16_t line, const std::string & message)
{
    if(level <= runtimeLevel)
    {
#ifdef LOG_INCLUDES_LOCATION
        std::cerr << toText(level) 
            << '\t' << file 
            << '\t' << function
            << '\t' << line
            << '\n' ; // the \n makes it harder to parse, but easier to read
#endif // LOG_INCLUDES_LOCATION
        std::cerr
            << message 
            << std::endl;
    }
}

void Log::setLevel(Level level)
{
    runtimeLevel = level;
}

const char * Log::toText(Level level)
{
    switch(level)
    {
        default:
            return "Unknown";
        case FORCE:
            return "FORCE";
        case FATAL:
            return "FATAL";
        case ERROR:
            return "ERROR";
        case WARNING:
            return "WARNING";
        case INFO:
            return "DEBUG";
        case DEBUG:
            return "TRACE";
        case TRACE:
            return "TRACE";
        case VERBOSE:
            return "VERBOSE";
    }
}
