#pragma once

#include "Core.h"
#include "Window.h"
#include "Event.h"
#include "Event/Handler.h"

namespace Surface {

	class SURF_API Application
	{
	public:
		Application();
		virtual ~Application();

		std::unique_ptr<Window> window;
		bool running = true;

		void OnEvent(Event& event);
		void Run();

		bool OnWindowClose(WindowClosedEvent& event);
	};

	Application* CreateApplication();

}
