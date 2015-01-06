// Copyright (c) 2009, Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifdef _MSC_VER
# pragma once
#endif
#ifndef HSQUEUE_EXPORT_H
#define HSQUEUE_EXPORT_H

// Compile time controls for library generation.  Define with /D or #define
// To produce or use a static library: #define HSQUEUE_HAS_DLL=0
//   Default is to produce/use a DLL
// While building the HSQUEUE_ library: #define HSQUEUE_BUILD_DLL
//   Default is to export symbols from a pre-built HSQueue dll
//
// Within HSQUEUE use the HSQueue_Export macro where a __declspec is needed.

#if defined (_WIN32)

#  if !defined (HSQUEUE_HAS_DLL)
#    define HSQUEUE_HAS_DLL 1
#  endif /* ! HSQUEUE_HAS_DLL */

#  if defined (HSQUEUE_HAS_DLL) && (HSQUEUE_HAS_DLL == 1)
#    if defined (HSQUEUE_BUILD_DLL)
#      define HSQueue_Export __declspec(dllexport)
#    else /* HSQUEUE_BUILD_DLL */
#      define HSQueue_Export __declspec(dllimport)
#    endif /* HSQUEUE_BUILD_DLL */
#  else /* HSQUEUE_HAS_DLL == 1 */
#    define HSQueue_Export
#  endif /* HSQUEUE_HAS_DLL == 1 */

#  else /* !_WIN32 */

#    define HSQueue_Export
#  endif /* _WIN32 */
#endif /* HSQUEUE_EXPORT_H */

