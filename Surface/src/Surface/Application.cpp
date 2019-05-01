#include "spch.h"

#include "Application.h"
#include <GLFW/glfw3.h>

namespace Surface {

#define BIND_APP_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application::Application()
	{
		window = std::unique_ptr<Window>(Window::Create());
		window->SetEventCallback(BIND_APP_FN(SendEvent));
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		while (running)
		{
			glClearColor(0.1, 0.5, 0.7, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			View* view = views[0];

			// Render layers first, in reverse order so that the first layer is above everything
			for (auto it = view->layers.end(); it != view->layers.begin();)
				(*--it)->Update();

			// Render overlays last, in reverse order as well
			for (auto it = view->overlays.end(); it != view->overlays.begin();)
				(*--it)->Update();

			window->OnUpdate();
		}
	}

	bool Application::AddView(View* view)
	{
		for (View* other : views)
		{
			if (other == view || other->name == view->name)
				return false;
		}
		views.push_back(view);
		return true;
	}

	void Application::RemoveView(View* view)
	{
		auto it = std::find(views.begin(), views.end(), view);
		if (it != views.end())
			views.erase(it);
	}

	void Application::RemoveView(const std::string& name)
	{
		auto index = views.begin();
		for (View* view : views)
		{
			if (view->name == name)
			{
				views.erase(index);
				return;
			}
			if (++index == views.end()) return;
		}
	}

	bool Application::SetView(View* view)
	{
		auto index = views.begin();
		for (View* other : views)
		{
			if (other == view)
			{
				if (index == views.begin())
					return true;
				views.erase(index);
				break;
			}
			else if (other->name == view->name)
				return false;
			if (++index == views.end()) break;
		}
		views.insert(views.begin(), view);
		return true;
	}

	bool Application::SetView(const std::string& name)
	{
		auto index = views.begin();
		for (View* view : views)
		{
			if (view->name == name)
			{
				if (index == views.begin())
					return true;
				views.erase(index);
				views.insert(views.begin(), view);
				return true;
			}
			if (++index == views.end()) break;
		}
		return false;
	}

	void Application::UnsetView(View* view)
	{
		auto it = std::find(views.begin(), views.end(), view);
		if (it == views.end() - 1 || it == views.end())
			return;
		views.erase(it);
		views.push_back(view);
	}

	void Application::UnsetView(const std::string& name)
	{
		auto index = views.begin();
		for (View* view : views)
		{
			if (view->name == name)
			{
				if (index == views.end() - 1)
					break;
				views.erase(index);
				views.push_back(view);
				break;
			}
			if (++index == views.end()) break;
		}
	}

	void Application::SendEvent(Event& event)
	{
		Handler handler(event);

		// Send event to sub-Application
		OnEvent(event);
		if (!event.active) return;

		View* view = views[0];

		// ALWAYS send event to overlays first, before base class
		for (Overlay* overlay : view->overlays)
		{
			if (event.active)
				overlay->SendEvent(event);
			else return;
		}

		// Send event to base Application functions
		handler.Fire<WindowClosedEvent>(BIND_APP_FN(WindowClose));

		// Send event to layers
		if (!event.active) return;

		for (Layer* layer : view->layers)
		{
			if (event.active)
				layer->SendEvent(event);
			else return;
		}
	}

	bool Application::WindowClose(WindowClosedEvent& event)
	{
		running = false;
		OnWindowClose(event);
		return true;
	}

#undef BIND_APP_FN

}
