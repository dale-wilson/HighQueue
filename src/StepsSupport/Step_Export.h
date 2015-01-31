// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
# pragma once

// Compile time controls for library generation.  Define with /D or #define
// To produce or use a static library: #define STEP_HAS_DLL=0
//   Default is to produce/use a DLL
// While building the Communication library: #define STEP_BUILD_DLL
//   Default is to export symbols from a pre-built HighQueue dll
//
// Within Communication use the Steps_Export macro where a __declspec is needed.

#if defined (_WIN32)

#  if !defined (STEP_HAS_DLL)
#    define STEP_HAS_DLL 1
#  endif /* ! STEP_HAS_DLL */

#  if defined (STEP_HAS_DLL) && (STEP_HAS_DLL == 1)
#    if defined (STEP_BUILD_DLL)
#      define Steps_Export __declspec(dllexport)
#    else /* STEP_BUILD_DLL */
#      define Steps_Export __declspec(dllimport)
#    endif /* STEP_BUILD_DLL */
#  else /* STEP_HAS_DLL == 1 */
#    define Steps_Export
#  endif /* STEP_HAS_DLL == 1 */

#  else /* !_WIN32 */

#    define Steps_Export
#  endif /* _WIN32 */

