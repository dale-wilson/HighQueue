// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Common/HighQueuePch.hpp>
#include "Log.hpp"

using namespace HighQueue;

#if 0 // VERBOSE
Log::Level Log::runtimeLevel = Log::Level(
    Log::FORCE | Log::FATAL | Log::ERROR | Log::WARNING | 
    Log::INFO | Log::STATISTICS | Log::TRACE | Log::DEBUG |
    Log::VERBOSE);
#elif defined(_DEBUG)
Log::Level Log::runtimeLevel = Log::Level(
    Log::FORCE | Log::FATAL | Log::ERROR | Log::WARNING | 
    Log::INFO | Log::STATISTICS | Log::TRACE | Log::DEBUG
    );
#else
Log::Level Log::runtimeLevel = Log::Level(
    Log::FORCE | Log::FATAL | Log::ERROR | Log::WARNING |
    Log::INFO | Log::STATISTICS);
#endif

void Log::log(Log::Level level, const char * file, const char * function, uint16_t line, const std::string & message)
{
    if((level & runtimeLevel) != 0)
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

std::string Log::toText(Level level)
{
    std::ostringstream str;
    if((level & FORCE) != 0)
    {
        str << " FORCE";
    }
    if((level & FATAL) != 0)
    {
        str << " FATAL";
    }
    if((level & ERROR) != 0)
    {
        str << " ERROR";
    }
    if((level & WARNING) != 0)
    {
        str << " WARNING";
    }
    if((level & INFO) != 0)
    {
        str << " DEBUG";
    }
    if((level & STATISTICS) != 0)
    {
        str << " STATISTICS";
    }
    if((level & DEBUG) != 0)
    {
        str << " TRACE";
    }
    if((level & TRACE) != 0)
    {
        str << " TRACE";
    }
    if((level & VERBOSE) != 0)
    {
        str << " VERBOSE";
    }
    return str.str();
}
