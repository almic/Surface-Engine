#pragma once

#include "Application.h"

#ifdef SURF_PLATFORM_WINDOWS

extern Surface::Application* Surface::CreateApplication(int arc, char** argv);

int main(int arc, char** argv)
{
	Surface::Log::Init();
	SURF_CORE_INFO("Starting App");

	Surface::Application* app = Surface::CreateApplication(arc, argv);
	app->Run();
	delete app;
}

#endif // SURF_PLATFORM_WINDOWS
