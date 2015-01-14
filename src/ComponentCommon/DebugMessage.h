#pragma once
#ifndef USE_DEBUG_MESSAGE
#define USE_DEBUG_MESSAGE 0
#endif

#if USE_DEBUG_MESSAGE
#define DebugMessage(TEXT) do{std::stringstream msg;msg << TEXT; std::cerr << msg.str();}while(false)
#else // USE_DEBUG_MESSAGE
#define DebugMessage(TEXT) do{;}while(false)
#endif // USE_DEBUG_MESSAGE


