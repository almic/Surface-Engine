#pragma once

#include "Core.h"
#include "Event.h"

struct GLFWwindow;

namespace Surface {

	struct SURF_API WindowProperties
	{
		using EventCallbackFunc = std::function<void(Event&)>;

		std::string title;
		unsigned int width;
		unsigned int height;
		unsigned int xPos;
		unsigned int yPos;
		bool vsync;
		int targetFPS;
		EventCallbackFunc eventCallback;

		WindowProperties(const std::string& title = "Surface",
		                 unsigned int width = 1920,
		                 unsigned int height = 1080,
						 unsigned int xPos = 0,
						 unsigned int yPos = 0,
			             bool vsync = true,
						 int targetFPS = 0)
			: title(title), width(width), height(height), xPos(xPos), yPos(yPos), vsync(vsync), targetFPS(targetFPS) {}

		static WindowProperties& GetProperties(GLFWwindow* window);

		// Changing these will not actually update the size or location of the window, this is only for the end application
		inline void SetSize(unsigned int width, unsigned int height) { this->width = width; this->height = height; }
		inline void SetPosition(unsigned int xPos, unsigned int yPos) { this->xPos = xPos; this->yPos = yPos; }
	};

	class SURF_API Window
	{
	public:
		using EventCallbackFunc = std::function<void(Event&)>;

		WindowProperties properties;

		virtual ~Window() {}

		static Window* Create(const WindowProperties& props = WindowProperties());
		virtual void Close() = 0;

		virtual GLFWwindow* GetGLFWwindow() = 0;
		virtual void OnUpdate() = 0;
		virtual void SetEventCallback(const EventCallbackFunc& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual void SetTargetFPS(int targetFPS) = 0;
		virtual bool IsVSync() const = 0;

		inline static void GLFWErrorCallback(int code, const char* reason) { SURF_CORE_ERROR("GLFW Error {0} {1}", code, reason); }
	};

}
