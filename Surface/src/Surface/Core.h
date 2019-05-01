#pragma once

#ifdef SURF_PLATFORM_WINDOWS

	#ifdef SURF_BUILD
		#define SURF_API __declspec(dllexport)
	#else
		#define SURF_API __declspec(dllimport)
	#endif // SURF_BUILD

#endif // SURF_PLATFORM_WINDOWS

#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
	#define ASSERT(x, ...) { if(!(x)) { SURF_ERROR("Assert failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define CORE_ASSERT(x, ...) { if(!(x)) { SURF_CORE_ERROR("Assert failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define ASSERT(x, ...)
	#define CORE_ASSERT(x, ...)
#endif
