#pragma once

#ifdef BLAM_PLATFORM_WINDOWS

	#ifdef BLAM_BUILD
		#define BLAM_API __declspec(dllexport)
	#else
		#define BLAM_API __declspec(dllimport)
	#endif // BLAM_BUILD

#endif // BLAM_PLATFORM_WINDOWS