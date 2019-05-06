#pragma once

#include "Surface/Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Surface {

	class Win64Window : public Window
	{
	public:
		Win64Window(const WindowProperties& props);
		virtual ~Win64Window();
		void Close() override;

		GLFWwindow* GetGLFWwindow() override { return window; }
		void OnUpdate() override;
		inline void SetEventCallback(const EventCallbackFunc& callback) override { properties.eventCallback = callback; };
		void SetVSync(bool enabled) override;
		void SetTargetFPS(int targetFPS) override;
		inline bool IsVSync() const override { return properties.vsync; };
	private:
		GLFWwindow* window;
	};

}
