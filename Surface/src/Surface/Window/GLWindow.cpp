#include "spch.h"

#include "GLWindow.h"

namespace Surface {

	static bool GLFWInitialized = false;

	Window* Window::Create(const WindowProperties& props)
	{
		return new GLWindow(props);
	}

	WindowProperties& WindowProperties::GetProperties(GLFWwindow* window)
	{
		return *(WindowProperties*)glfwGetWindowUserPointer(window);
	}

	GLWindow::GLWindow(const WindowProperties& props)
	{
		properties = props;

		SURF_CORE_INFO("Creating OpenGL window \"{0}\" ({1}, {2})", properties.title, properties.width, properties.height);

		if (!GLFWInitialized)
		{
			int success = glfwInit();
			CORE_ASSERT(success, "Unable to initialize GLFW");
			GLFWInitialized = true;
		}

		glfwSetErrorCallback(GLFWErrorCallback);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

		if (properties.form == WindowForm::FULLSCREEN)
			window = glfwCreateWindow(properties.width, properties.height, properties.title.c_str(), glfwGetPrimaryMonitor(), nullptr);
		else
			window = glfwCreateWindow(properties.width, properties.height, properties.title.c_str(), nullptr, nullptr);
		
		SURF_CORE_INFO("Using OpenGL version {0}.{1}", glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR), glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR));

		glfwMakeContextCurrent(window);
		glfwSetWindowUserPointer(window, &properties);
		glfwSwapInterval(properties.vsync);


		if (properties.form == WindowForm::HIDDEN)
			glfwHideWindow(window);
		else if (properties.form == WindowForm::MINIMIZED)
			glfwIconifyWindow(window);
		else if (properties.form == WindowForm::MAXIMIZED)
			glfwMaximizeWindow(window);
		else if (properties.form == WindowForm::BORDERLESS)
			glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
		else if (properties.form == WindowForm::WINDOWED_FULLSCREEN)
		{
			glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);

			// Store primary monitor coords
			int xpos, ypos, width, height;
			glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &xpos, &ypos, &width, &height);

			glfwSetWindowPos(window, xpos, ypos);
			glfwSetWindowSize(window, width, height);
		}

		if (properties.setPosition)
			glfwSetWindowPos(window, (int)properties.xPos, (int)properties.yPos);

		// Update window size data again, anything could've happened by now
		{
			int width, height;
			glfwGetWindowSize(window, &width, &height);
			properties.SetSize((unsigned int)width, (unsigned int)height);
		}

		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		CORE_ASSERT(status, "Unable to initialize Glad");

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

		glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int character)
		{
			WindowProperties& props = WindowProperties::GetProperties(window);

			props.eventCallback(CharacterInputEvent(character));
		});
	}

	GLWindow::~GLWindow()
	{
		Close();
	}

	void GLWindow::Close()
	{
		glfwDestroyWindow(window);
	}

	void GLWindow::OnUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	void GLWindow::SetSize(unsigned int width, unsigned int height)
	{
		glfwSetWindowSize(window, (int)width, (int)height);
	}

	void GLWindow::SetWindowForm(WindowForm form)
	{
		#define FORCE_WINDOWED(x) if (!!glfwGetWindowMonitor(x)) glfwSetWindowMonitor(x, NULL, (int)properties.xPos, (int)properties.yPos, (int)properties.width, (int)properties.height, 0)
		switch (form)
		{
			case WindowForm::HIDDEN:
			{
				FORCE_WINDOWED(window);
				glfwHideWindow(window);
				break;
			}
			case WindowForm::MINIMIZED:
			{
				glfwIconifyWindow(window);
				break;
			}
			case WindowForm::WINDOWED:
			{
				FORCE_WINDOWED(window);
				break;
			}
			case WindowForm::MAXIMIZED:
			{
				FORCE_WINDOWED(window);
				glfwMaximizeWindow(window);
				break;
			}
			case WindowForm::BORDERLESS:
			{
				FORCE_WINDOWED(window);
				glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
				break;
			}
			case WindowForm::WINDOWED_FULLSCREEN:
			{
				FORCE_WINDOWED(window);
				glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);

				// Store primary monitor coords
				int xpos, ypos, width, height;
				glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &xpos, &ypos, &width, &height);

				glfwSetWindowPos(window, xpos, ypos);
				glfwSetWindowSize(window, width, height);
				break;
			}
			case WindowForm::FULLSCREEN:
			{
				glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, (int)properties.width, (int)properties.height, GLFW_DONT_CARE);
				break;
			}
			case WindowForm::RESTORE:
			default:
			{
				glfwRestoreWindow(window);
			}
		}
		#undef FORCE_WINDOWED
	}

	void GLWindow::SetVSync(bool enabled)
	{
		if (enabled != properties.vsync)
		{
			properties.vsync = enabled;
			glfwSwapInterval(properties.vsync);
		}
	}

	void GLWindow::SetTargetFPS(int targetFPS)
	{
		if (targetFPS > 0)
		{
			SetVSync(false);
			properties.targetFPS = targetFPS;
		}
	}

}
