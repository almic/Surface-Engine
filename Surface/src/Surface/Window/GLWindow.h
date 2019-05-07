#pragma once

#include "Surface/Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Surface {

	class GLWindow : public Window
	{
	public:
		GLWindow(const WindowProperties& props);
		virtual ~GLWindow();
		void Close() override;

		GLFWwindow* GetGLFWwindow() override { return window; }
		void OnUpdate() override;
		inline void SetEventCallback(const EventCallbackFunc& callback) override { properties.eventCallback = callback; };
		void SetSize(unsigned int width, unsigned int height) override;
		void SetWindowForm(WindowForm form) override;
		void SetVSync(bool enabled) override;
		void SetTargetFPS(int targetFPS) override;
		inline bool IsVSync() const override { return properties.vsync; };
	private:
		GLFWwindow* window;
	};

}
