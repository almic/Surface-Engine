#include "spch.h"

#include "Application.h"
#include <GLFW/glfw3.h>

namespace Surface {

#define BIND_APP_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application::Application()
	{
		window = std::unique_ptr<Window>(Window::Create());
		window->SetEventCallback(BIND_APP_FN(OnEvent));
	}

	Application::~Application()
	{
	}

	void Application::OnEvent(Event& event)
	{
		//SURF_CORE_TRACE(event);

		Handler handler(event);
		handler.Fire<WindowClosedEvent>(BIND_APP_FN(OnWindowClose));
	}

	void Application::Run()
	{
		while (running)
		{
			glClearColor(0.1, 0.5, 0.7, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowClosedEvent& event)
	{
		running = false;
		return true;
	}

#undef BIND_APP_FN

}
