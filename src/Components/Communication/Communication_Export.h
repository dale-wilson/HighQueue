// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
# pragma once

// Compile time controls for library generation.  Define with /D or #define
// To produce or use a static library: #define COMMUNICATION_HAS_DLL=0
//   Default is to produce/use a DLL
// While building the Communication library: #define COMMUNICATION_BUILD_DLL
//   Default is to export symbols from a pre-built HighQueue dll
//
// Within Communication use the Communication_Export macro where a __declspec is needed.

#if defined (_WIN32)

#  if !defined (COMMUNICATION_HAS_DLL)
#    define COMMUNICATION_HAS_DLL 1
#  endif /* ! COMMUNICATION_HAS_DLL */

#  if defined (COMMUNICATION_HAS_DLL) && (COMMUNICATION_HAS_DLL == 1)
#    if defined (COMMUNICATION_BUILD_DLL)
#      define Communication_Export __declspec(dllexport)
#    else /* COMMUNICATION_BUILD_DLL */
#      define Communication_Export __declspec(dllimport)
#    endif /* COMMUNICATION_BUILD_DLL */
#  else /* COMMUNICATION_HAS_DLL == 1 */
#    define Communication_Export
#  endif /* COMMUNICATION_HAS_DLL == 1 */

#  else /* !_WIN32 */

#    define Communication_Export
#  endif /* _WIN32 */

