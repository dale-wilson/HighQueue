// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
# pragma once

// Compile time controls for library generation.  Define with /D or #define
// To produce or use a static library: #define HIGHQUEUE_HAS_DLL=0
//   Default is to produce/use a DLL
// While building the HIGHQUEUE_ library: #define HIGHQUEUE_BUILD_DLL
//   Default is to export symbols from a pre-built HighQueue dll
//
// Within HIGHQUEUE use the HighQueue_Export macro where a __declspec is needed.

#if defined (_WIN32)

#  if !defined (HIGHQUEUE_HAS_DLL)
#    define HIGHQUEUE_HAS_DLL 1
#  endif /* ! HIGHQUEUE_HAS_DLL */

#  if defined (HIGHQUEUE_HAS_DLL) && (HIGHQUEUE_HAS_DLL == 1)
#    if defined (HIGHQUEUE_BUILD_DLL)
#      define HighQueue_Export __declspec(dllexport)
#    else /* HIGHQUEUE_BUILD_DLL */
#      define HighQueue_Export __declspec(dllimport)
#    endif /* HIGHQUEUE_BUILD_DLL */
#  else /* HIGHQUEUE_HAS_DLL == 1 */
#    define HighQueue_Export
#  endif /* HIGHQUEUE_HAS_DLL == 1 */

#  else /* !_WIN32 */

#    define HighQueue_Export
#  endif /* _WIN32 */
