// Copyright (c) 2009, Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifdef _MSC_VER
# pragma once
#endif
#ifndef PRONTOQUEUE_EXPORT_H
#define PRONTOQUEUE_EXPORT_H

// Compile time controls for library generation.  Define with /D or #define
// To produce or use a static library: #define PRONTOQUEUE_HAS_DLL=0
//   Default is to produce/use a DLL
// While building the PRONTOQUEUE_ library: #define PRONTOQUEUE_BUILD_DLL
//   Default is to export symbols from a pre-built ProntoQueue dll
//
// Within PRONTOQUEUE use the ProntoQueue_Export macro where a __declspec is needed.

#if defined (_WIN32)

#  if !defined (PRONTOQUEUE_HAS_DLL)
#    define PRONTOQUEUE_HAS_DLL 1
#  endif /* ! PRONTOQUEUE_HAS_DLL */

#  if defined (PRONTOQUEUE_HAS_DLL) && (PRONTOQUEUE_HAS_DLL == 1)
#    if defined (PRONTOQUEUE_BUILD_DLL)
#      define ProntoQueue_Export __declspec(dllexport)
#    else /* PRONTOQUEUE_BUILD_DLL */
#      define ProntoQueue_Export __declspec(dllimport)
#    endif /* PRONTOQUEUE_BUILD_DLL */
#  else /* PRONTOQUEUE_HAS_DLL == 1 */
#    define ProntoQueue_Export
#  endif /* PRONTOQUEUE_HAS_DLL == 1 */

#  else /* !_WIN32 */

#    define ProntoQueue_Export
#  endif /* _WIN32 */
#endif /* PRONTOQUEUE_EXPORT_H */

