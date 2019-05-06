#pragma once

#include "Core.h"
#include "Window.h"
#include "Event.h"
#include "Event/Handler.h"
#include "View.h"
#include <chrono>

#define MICRO_TIME std::chrono::time_point<std::chrono::steady_clock>
#define MICRO_CLOCK std::chrono::high_resolution_clock
#define MICRO_CLOCK_DURATION std::chrono::duration_cast<std::chrono::microseconds>

namespace Surface {

	class SURF_API Application
	{
	public:
		Application();
		virtual ~Application();
		
		/*static Application* app;
		inline static Application& Get() { return *app; }*/

		bool running = true;
		
		double deltaTime = 1.0 / 60.0; // Default for when app starts
		MICRO_TIME tickStart;
		MICRO_TIME tickEnd;

		std::unique_ptr<Window> window;
		std::vector<View*> views;

		virtual void OnEvent(Event& event) {}
		virtual void OnTick(const double &deltaTime) {}
		virtual void OnWindowClose(WindowClosedEvent& event) {}

		virtual void Run() final;
		virtual void StartTick() final;
		virtual void EndTick() final;

		// Adds views, ordered such that the first added is the active view.
		// No two views may share the same name!! Returns true if the view was added, false if one with
		// the same name already exists.
		virtual bool AddView(View* view) final;

		// Removes the view from the Application, it is recommended to explicitly set the active view
		// after calling this because the order could have changed since.
		virtual void RemoveView(View* view) final;
		virtual void RemoveView(const std::string& name) final;

		// Sets the view passed as the active view. If it does not exist in the list, it is added.
		virtual bool SetView(View* view) final;
		virtual bool SetView(const std::string& name) final;

		// Moves the view passed to the end of the list so a different view can be active.
		// You should really just use SetView() instead, but this could be helpful to someone
		virtual void UnsetView(View* view) final;
		virtual void UnsetView(const std::string& name) final;

		virtual void SendEvent(Event& event) final;
		virtual bool WindowClose(WindowClosedEvent& event) final;
	};

	Application* CreateApplication(int arc, char** argv);

}
