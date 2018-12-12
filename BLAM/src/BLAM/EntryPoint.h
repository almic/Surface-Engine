#pragma once

#include "Application.h"

#ifdef BLAM_PLATFORM_WINDOWS

extern Blam::Application* Blam::CreateApplication();

int main(int arc, char** argv)
{
	Blam::Log::Init();
	BLAM_CORE_INFO("Hello there!");

	auto app = Blam::CreateApplication();
	app->Run();
	delete app;
}

#endif // BLAM_PLATFORM_WINDOWS