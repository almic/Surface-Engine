#pragma once

#include "Core.h"
#include "Event.h"

struct GLFWwindow;

namespace Surface {

	enum class WindowForm : unsigned
	{
		HIDDEN,
		MINIMIZED,
		WINDOWED,
		MAXIMIZED,
		BORDERLESS,
		WINDOWED_FULLSCREEN,
		FULLSCREEN,
		RESTORE
	};

	struct SURF_API WindowProperties
	{
		using EventCallbackFunc = std::function<void(Event&)>;

		std::string title;
		unsigned int width;
		unsigned int height;
		unsigned int xPos;
		unsigned int yPos;
		bool setPosition;
		bool vsync;
		int targetFPS;
		WindowForm form;
		EventCallbackFunc eventCallback;

		WindowProperties(const std::string& title = "Surface",
		                 unsigned int width = 853,
		                 unsigned int height = 480,
						 int xPos = 0,
						 int yPos = 0,
						 bool setPosition = false,
			             bool vsync = true,
						 int targetFPS = 0,
						 WindowForm form = WindowForm::WINDOWED)
			: title(title), width(width), height(height), xPos(xPos), yPos(yPos), setPosition(setPosition), vsync(vsync), targetFPS(targetFPS), form(form) {}

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

		static Window* Create(const WindowProperties& props);
		virtual void Close() = 0;

		virtual GLFWwindow* GetGLFWwindow() = 0;
		virtual void OnUpdate() = 0;
		virtual void SetEventCallback(const EventCallbackFunc& callback) = 0;
		virtual void SetSize(unsigned int width, unsigned int height) = 0;
		virtual void SetWindowForm(WindowForm form) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual void SetTargetFPS(int targetFPS) = 0;
		virtual bool IsVSync() const = 0;

		inline static void GLFWErrorCallback(int code, const char* reason) { SURF_CORE_ERROR("GLFW Error {0} {1}", code, reason); }
	};

}
