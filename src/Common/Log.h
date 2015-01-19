// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Common/HighQueue_Export.h>

// NOTE: To avoid external dependancies on any of the existing logging packages (there are many)
// I defined a simple logging system invoked via macros with the hopes that one could implement those
// macros around the desired logging.  (This may be a case of NIH)

// Give MSVC a hint about the c++11 standard macro:
#if defined(_MSC_VER) && ! defined(__func__)
#define __func__ __FUNCTION__
#endif // _MSC_VER

#define LOG_LEVEL_FORCE 0
#define LOG_LEVEL_FATAL 1
#define LOG_LEVEL_ERROR 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_INFO 4
#define LOG_LEVEL_TRACE 5
#define LOG_LEVEL_DEBUG 6
#define LOG_LEVEL_VERBOSE 7

#if !defined(COMPILE_TIME_LOG_LEVEL)
#   if defined(_DEBUG)
#       define COMPILE_TIME_LOG_LEVEL LOG_LEVEL_VERBOSE
#   else
#       define COMPILE_TIME_LOG_LEVEL LOG_LEVEL_ERROR
#   endif // _DEBUG
#endif COMPILE_TIME_LOG_LEVEL

namespace HighQueue
{
    struct HighQueue_Export Log
    {
        enum Level: byte_t
        {
            FORCE,
            FATAL,
            ERROR,
            WARNING,
            INFO,
            TRACE,
            DEBUG,
            VERBOSE
        };

        static Level runtimeLevel;
        static void log(Level level, const char * file, const char * function, uint16_t line, const std::string & message);
        static bool isEnabled(Level level);
        static Level getLevel();
        static void setLevel(Level level);
        static const char * toText(Level level);
    };

    inline
    bool Log::isEnabled(Level level)
    {
        return level <= runtimeLevel;
    }

    inline
    Log::Level Log::getLevel()
    {
        return runtimeLevel;
    }

#define LogForce(message) do{std::stringstream msg; msg << message; Log::log(Log::FORCE, __FILE__, __func__, __LINE__, msg.str());}while(false)
#define LogFatal(message) do{if(Log::isEnabled(Log::FATAL)){std::stringstream msg; msg << message; Log::log(Log::FATAL, __FILE__, __func__, __LINE__, msg.str());}}while(false)
#define LogError(message) do{if(Log::isEnabled(Log::ERROR)){std::stringstream msg; msg << message; Log::log(Log::ERROR, __FILE__, __func__, __LINE__, msg.str());}}while(false)

#if (LOG_LEVEL_WARNING <= COMPILE_TIME_LOG_LEVEL)
#define LogWarning(message) do{if(Log::isEnabled(Log::WARNING)){std::stringstream msg; msg << message; Log::log(Log::WARNING, __FILE__, __func__, __LINE__, msg.str());}}while(false)
#else // LOG_LEVEL_WARNING 
#define LogWarning(message) do{}while(false)
#endif // LOG_LEVEL_WARNING 

#if (LOG_LEVEL_INFO <= COMPILE_TIME_LOG_LEVEL)
#define LogInfo(message) do{if(Log::isEnabled(Log::INFO)){std::stringstream msg; msg << message; Log::log(Log::INFO, __FILE__, __func__, __LINE__, msg.str());}}while(false)
#else // LOG_LEVEL_INFO 
#define LogInfo(message) do{}while(false)
#endif // LOG_LEVEL_INFO 
#if (LOG_LEVEL_DEBUG <= COMPILE_TIME_LOG_LEVEL)
#define LogDebug(message) do{if(Log::isEnabled(Log::DEBUG)){std::stringstream msg; msg << message; Log::log(Log::DEBUG, __FILE__, __func__, __LINE__, msg.str());}}while(false)
#else // LOG_LEVEL_DEBUG 
#define LogDebug(message) do{}while(false)
#endif // LOG_LEVEL_DEBUG 
#if (LOG_LEVEL_TRACE <= COMPILE_TIME_LOG_LEVEL)
#define LogTrace(message) do{if(Log::isEnabled(Log::TRACE)){std::stringstream msg; msg << message; Log::log(Log::TRACE, __FILE__, __func__, __LINE__, msg.str());}}while(false)
#else // LOG_LEVEL_TRACE 
#define LogTrace(message) do{}while(false)
#endif // LOG_LEVEL_TRACE 
#if (LOG_LEVEL_VERBOSE <= COMPILE_TIME_LOG_LEVEL)
#define LogVerbose(message) do{if(Log::isEnabled(Log::VERBOSE)){std::stringstream msg; msg << message; Log::log(Log::VERBOSE, __FILE__, __func__, __LINE__, msg.str());}}while(false)
#else // LOG_LEVEL_VERBOSE 
#define LogVerbose(message) do{}while(false)
#endif // LOG_LEVEL_VERBOSE 

} // namespace HighQueue
