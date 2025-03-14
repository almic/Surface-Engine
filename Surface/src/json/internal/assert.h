#pragma once

// Assert macro
#ifdef DEBUG

#ifdef PLATFORM_WINDOWS
#define assert(expression) (void) (!!(expression) || (__debugbreak(), 0))

#else // unknown platform
#define assert(expression)
#endif

#else // not in debug
#define assert(expression)
#endif
