#include "spch.h"

#include "Win64Window.h"
#include <GLFW/glfw3.h>

namespace Surface {

	static bool GLFWInitialized = false;

	Window* Window::Create(const WindowProperties& props)
	{
		return new Win64Window(props);
	}

	WindowProperties& WindowProperties::GetProperties(GLFWwindow* window)
	{
		return *(WindowProperties*)glfwGetWindowUserPointer(window);
	}

	Win64Window::Win64Window(const WindowProperties& props)
	{
		properties = props;

		SURF_CORE_INFO("Creating window \"{0}\" ({1}, {2})", properties.title, properties.width, properties.height);

		if (!GLFWInitialized)
		{
			int success = glfwInit();
			CORE_ASSERT(success, "Unable to initialize GLFW");
			GLFWInitialized = true;
		}

		window = glfwCreateWindow(properties.width, properties.height, properties.title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(window);
		glfwSetWindowUserPointer(window, &properties);
		glfwSwapInterval(properties.vsync);
		glfwSetErrorCallback(GLFWErrorCallback);

		// Window callbacks
		glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int hasFocus)
		{
			WindowProperties& props = WindowProperties::GetProperties(window);

			if (hasFocus) {
				props.eventCallback(WindowFocusedEvent());
			} else {
				props.eventCallback(WindowLostFocusEvent());
			}
		});

		glfwSetWindowPosCallback(window, [](GLFWwindow* window, int xPos, int yPos)
		{
			WindowProperties& props = WindowProperties::GetProperties(window);
			props.SetPosition(xPos, yPos);

			props.eventCallback(WindowMovedEvent(xPos, yPos));
		});

		glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height)
		{
			WindowProperties& props = WindowProperties::GetProperties(window);
			props.SetSize(width, height);

			props.eventCallback(WindowResizedEvent(width, height));
		});

		glfwSetWindowCloseCallback(window, [](GLFWwindow* window)
		{
			WindowProperties& props = WindowProperties::GetProperties(window);

			props.eventCallback(WindowClosedEvent());
		});

		// Input callbacks
		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowProperties& props = WindowProperties::GetProperties(window);

			props.eventCallback(MouseMovedEvent((int)xPos, (int)yPos));
		});

		glfwSetScrollCallback(window, [](GLFWwindow* window, double right, double up)
		{
			WindowProperties& props = WindowProperties::GetProperties(window);

			props.eventCallback(MouseScrolledEvent((int)up, (int)right));
		});

		glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowProperties& props = WindowProperties::GetProperties(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					props.eventCallback(MouseButtonPressedEvent(button));
					break;
				}
				case GLFW_RELEASE:
				{
					props.eventCallback(MouseButtonReleasedEvent(button));
					break;
				}
			}
		});

		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowProperties& props = WindowProperties::GetProperties(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					props.eventCallback(KeyPressedEvent(key, 0));
					break;
				}
				case GLFW_RELEASE:
				{
					props.eventCallback(KeyReleasedEvent(key));
					break;
				}
				case GLFW_REPEAT:
				{
					props.eventCallback(KeyPressedEvent(key, 1));
					break;
				}
			}
		});
	}

	Win64Window::~Win64Window()
	{
		Close();
	}

	void Win64Window::Close()
	{
		glfwDestroyWindow(window);
	}

	void Win64Window::OnUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	void Win64Window::SetVSync(bool enabled)
	{
		if (enabled != properties.vsync)
		{
			properties.vsync = enabled;
			glfwSwapInterval(properties.vsync);
		}
	}

}
