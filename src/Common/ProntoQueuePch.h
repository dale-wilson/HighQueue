// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

// Suppress spurious Visual C++ warnings
#if defined(_MSC_VER)
# pragma warning(disable:4251) // Disable VC warning about dll linkage required (for private members?)
# pragma warning(disable:4275) // disable warning about non dll-interface base class.
# pragma warning(disable:4996) // Disable VC warning that std library may be unsafe
# pragma warning(disable:4290) // C4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
# pragma warning(disable:4396) // Disable 'boost::operator !=' : the inline specifier cannot be used when a friend declaration refers to a specialization of a function template
// boost::unordered_set triggers this.  I think it's a bug somewhere, but it doesn't
// cause any problems because the code never compares boost::unordered sets
#pragma warning(disable:4820)  // 'n' bytes padding added after data member
#pragma warning(disable:4127)  // Conditonal expression is constant (particularly in templates)
#pragma warning(disable:4100)  // Disable: unreferenced formal parameter (/W4 warning: common case for virtual methods)
#endif

// Define compiler-specific macros to cache-align structures
// Usage:
//     PRE_CACHE_ALIGN
//     struct abc {
//     } POST_CACHE_ALIGN;
const size_t CacheLineSize = 64;
#if defined(_MSC_VER)
#	define PRE_CACHE_ALIGN __declspec(align(64))
#   define POST_CACHE_ALIGN
#elif // gcc?
#   define PRE_CACHE_ALIGN
#	define POST_CACHE_ALIGN __attribute__ ((aligned (64)))
#endif _MSC_VER

// If this symbol is not defined the user included a ProntoQueue header without
// using one of the standard precompiled header files.
#define ProntoQueue_HEADERS

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN      // Exclude rarely-used stuff from Windows headers
# define NOMINMAX                 // Do not define min & max a macros: l'histoire anciene
# include <windows.h>
// If building for .NET, must link boost threads dynamically
#define BOOST_THREAD_USE_DLL
#endif // _WIN32
// This reports at compile time which boost libraries will be used
// #define BOOST_LIB_DIAGNOSTIC

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <stack>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>


#ifdef NEED_BOOST_YET
#include <boost/shared_array.hpp>
#include <boost/chrono.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/operators.hpp>
#include <boost/thread.hpp>
#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#endif // NEED_BOOST_YET

typedef uint8_t byte_t;


