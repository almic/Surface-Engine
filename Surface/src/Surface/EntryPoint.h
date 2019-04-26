#pragma once

#include "Application.h"

#ifdef SURF_PLATFORM_WINDOWS

extern Surface::Application* Surface::CreateApplication();

int main(int arc, char** argv)
{
	Surface::Log::Init();
	SURF_CORE_INFO("Hello there!");

	auto app = Surface::CreateApplication();
	app->Run();
	delete app;
}

#endif // SURF_PLATFORM_WINDOWS
