#include "spch.h"

#include "Surface/Application.h"
#include "Surface/Input.h"
#include <GLFW/glfw3.h>

namespace Surface {

	#define APPWINDOW Application::GetApp()->window->GetGLFWwindow()

	bool Input::IsKeyPressed(int key)
	{
		auto state = glfwGetKey(APPWINDOW, key);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsCursorButtonPressed(int button)
	{
		return glfwGetMouseButton(APPWINDOW, button) == GLFW_PRESS;
	}

	std::pair<float, float> Input::GetCursorXY()
	{
		double x, y;
		glfwGetCursorPos(APPWINDOW, &x, &y);
		return { (float)x, (float)y };
	}

	#undef APPWINDOW
}
