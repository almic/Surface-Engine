#pragma once

#include "Surface/Window.h"

struct GLFWwindow;

namespace Surface {

	class Win64Window : public Window
	{
	public:
		Win64Window(const WindowProperties& props);
		virtual ~Win64Window();
		void Close() override;

		void OnUpdate() override;
		inline void SetEventCallback(const EventCallbackFunc& callback) override { properties.eventCallback = callback; };
		void SetVSync(bool enabled) override;
		inline bool IsVSync() const override { return properties.vsync; };
	private:
		GLFWwindow* window;
	};

}
