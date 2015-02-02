// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Common/HighQueue_Export.h>

// NOTE: To avoid external dependancies on any of the existing logging packages (there are many)
// I defined a simple logging system invoked via macros with the hopes that one could implement those
// macros around the desired logging.  (This may be a case of NIH)

// Give MSVC a clue about the c++11 standard macro:
#if defined(_MSC_VER) && ! defined(__func__)
#define __func__ __FUNCTION__
#endif // _MSC_VER

#define LOG_LEVEL_FORCE 0x0001
#define LOG_LEVEL_FATAL 0x0002
#define LOG_LEVEL_ERROR 0x0004
#define LOG_LEVEL_WARNING 0x0008
#define LOG_LEVEL_INFO 0x0010
#define LOG_LEVEL_STATISTICS 0x0020
#define LOG_LEVEL_TRACE 0x0040
#define LOG_LEVEL_DEBUG 0x0080
#define LOG_LEVEL_VERBOSE 0x0100

#if !defined(COMPILE_TIME_LOG_LEVEL)
#   if defined(_DEBUG)
#       define COMPILE_TIME_LOG_LEVEL (LOG_LEVEL_FORCE | LOG_LEVEL_FATAL | LOG_LEVEL_ERROR | LOG_LEVEL_WARNING | \
                                      LOG_LEVEL_INFO | LOG_LEVEL_STATISTICS | LOG_LEVEL_TRACE | LOG_LEVEL_DEBUG)
#   else
#       define COMPILE_TIME_LOG_LEVEL (LOG_LEVEL_FORCE | LOG_LEVEL_FATAL | LOG_LEVEL_ERROR | LOG_LEVEL_WARNING | LOG_LEVEL_STATISTICS)
#   endif // _DEBUG
#endif COMPILE_TIME_LOG_LEVEL

namespace HighQueue
{
    struct HighQueue_Export Log
    {
        enum Level: uint16_t
        {
            FORCE = LOG_LEVEL_FORCE,
            FATAL = LOG_LEVEL_FATAL,
            ERROR = LOG_LEVEL_ERROR,
            WARNING = LOG_LEVEL_WARNING,
            INFO = LOG_LEVEL_INFO,
            STATISTICS = LOG_LEVEL_STATISTICS,
            TRACE = LOG_LEVEL_TRACE,
            DEBUG = LOG_LEVEL_DEBUG,
            VERBOSE = LOG_LEVEL_VERBOSE
        };

        static Level runtimeLevel;
        static void log(Level level, const char * file, const char * function, uint16_t line, const std::string & message);
        static bool isEnabled(Level level);
        static Level getLevel();
        static void setLevel(Level level);
        static std::string toText(Level level);
    };

    inline
    bool Log::isEnabled(Level level)
    {
        return (level & runtimeLevel) != 0;
    }

    inline
    Log::Level Log::getLevel()
    {
        return runtimeLevel;
    }


#define LogTyped(type, message)\
    do{\
        std::stringstream msg;\
        msg << message;\
        Log::log(Log::type, __FILE__, __func__, __LINE__, msg.str());\
    }while(false)

#define LogTypedLimited(type, limit, message)\
    do{\
        static size_t remaining__ = limit;\
        if(remaining__ > 0){\
            --remaining__;\
            std::stringstream msg__;\
            msg__ << (remaining__ == 0 ? "Last time: " : "") << message;\
            Log::log(Log::type, __FILE__, __func__, __LINE__, msg__.str());\
            }\
        }while(false)

#define LogDisabled do{}while(false)

#define LogForce(message) LogTyped(FORCE, message)
#define LogForceLimited(limit, message) LogTypedLimited(FORCE, limit, message)

#define LogFatal(message) LogTyped(FATAL, message)
#define LogFatalLimited(limit, message) LogTypedLimited(FATAL, limit, message)

#define LogError(message) LogTyped(ERROR, message)
#define LogErrorLimited(limit, message) LogTypedLimited(ERROR, limit, message)

#if (LOG_LEVEL_WARNING & COMPILE_TIME_LOG_LEVEL)
#define LogWarning(message) LogTyped(WARNING, message)
#define LogWarningLimited(limit, message) LogTypedLimited(WARNING, limit, message)
#else // LOG_LEVEL_WARNING 
#define LogWarning(message) LogDisabled
#define LogWarningLimited(limited, message) LogDisabled
#endif // LOG_LEVEL_WARNING 

#if (LOG_LEVEL_INFO & COMPILE_TIME_LOG_LEVEL)
#define LogInfo(message) LogTyped(INFO, message)
#define LogInfoLimited(limit, message) LogTypedLimited(INFO, limit, message)
#else // LOG_LEVEL_INFO 
#define LogInfo(message) LogDisabled
#define LogInfoLimited(limit, message) LogDisabled
#endif // LOG_LEVEL_INFO 

#if (LOG_LEVEL_STATISTICS & COMPILE_TIME_LOG_LEVEL)
#define LogStatistics(message) LogTyped(STATISTICS, message)
#define LogStatisticsLimited(limit, message) LogTypedLimited(STATISTICS, limit, message)
#else // LOG_LEVEL_STATISTICS 
#define LogStatistics(message) LogDisabled
#define LogStatisticsLimited(limit, message) LogDisabled
#endif // LOG_LEVEL_STATISTICS 

#if (LOG_LEVEL_DEBUG & COMPILE_TIME_LOG_LEVEL)
#define LogDebug(message) LogTyped(DEBUG, message)
#define LogDebugLimited(limit, message) LogTypedLimited(DEBUG, limit, message)
#else // LOG_LEVEL_DEBUG 
#define LogDebug(message) LogDisabled
#define LogDebugLimited(limit, message) LogDisabled
#endif // LOG_LEVEL_DEBUG 

#if (LOG_LEVEL_TRACE & COMPILE_TIME_LOG_LEVEL)
#define LogTrace(message) LogTyped(TRACE, message)
#define LogTraceLimited(limit, message) LogTypedLimited(TRACE, limit, message)
#else // LOG_LEVEL_TRACE 
#define LogTrace(message) LogDisabled
#define LogTraceLimited(limit, message) LogDisabled
#endif // LOG_LEVEL_TRACE 

#if (LOG_LEVEL_VERBOSE & COMPILE_TIME_LOG_LEVEL)
#define LogVerbose(message) LogTyped(VERBOSE, message)
#define LogVerboseLimited(limit, message) LogTypedLimited(VERBOSE, limit, message)
#else // LOG_LEVEL_VERBOSE 
#define LogVerbose(message) LogDisabled
#define LogVerboseLimited(limit, message) LogDisabled
#endif // LOG_LEVEL_VERBOSE 

} // namespace HighQueue
