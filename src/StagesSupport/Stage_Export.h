// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
# pragma once

// Compile time controls for library generation.  Define with /D or #define
// To produce or use a static library: #define STAGE_HAS_DLL=0
//   Default is to produce/use a DLL
// While building the Communication library: #define STAGE_BUILD_DLL
//   Default is to export symbols from a pre-built HighQueue dll
//
// Within Communication use the Stages_Export macro where a __declspec is needed.

#if defined (_WIN32)

#  if !defined (STAGE_HAS_DLL)
#    define STAGE_HAS_DLL 1
#  endif /* ! STAGE_HAS_DLL */

#  if defined (STAGE_HAS_DLL) && (STAGE_HAS_DLL == 1)
#    if defined (STAGE_BUILD_DLL)
#      define Stages_Export __declspec(dllexport)
#    else /* STAGE_BUILD_DLL */
#      define Stages_Export __declspec(dllimport)
#    endif /* STAGE_BUILD_DLL */
#  else /* STAGE_HAS_DLL == 1 */
#    define Stages_Export
#  endif /* STAGE_HAS_DLL == 1 */

#  else /* !_WIN32 */

#    define Stages_Export
#  endif /* _WIN32 */

