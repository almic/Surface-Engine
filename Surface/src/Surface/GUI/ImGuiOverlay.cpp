#include "spch.h"

#include "ImGuiOverlay.h"

namespace Surface {

#define BIND_GUI_FN(x) std::bind(&ImGuiOverlay::x, this, std::placeholders::_1)

	GLFWcursor* ImGuiOverlay::cursors[ImGuiMouseCursor_COUNT] = { 0 };
	bool ImGuiOverlay::mouseJustPressed[5] = { false, false, false, false, false };

	ImGuiOverlay::ImGuiOverlay(const std::string& name)
			: Overlay(name)
	{
	}

	void ImGuiOverlay::ShowGui()
	{
		ImGui::ShowDemoWindow(&show);
	}

	void ImGuiOverlay::OnAttach()
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.BackendPlatformName = "Surface::ImGuiOverlay";

		// Keyboard mapping
		io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
		io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
		io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
		io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
		io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
		io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
		io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
		io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
		io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
		io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
		io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

		io.SetClipboardTextFn = [](void* window, const char* text) { glfwSetClipboardString((GLFWwindow*)window, text); };
		io.GetClipboardTextFn = [](void* window) { return glfwGetClipboardString((GLFWwindow*)window); };
		io.ClipboardUserData = app->window->GetGLFWwindow();

		cursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		cursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		cursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);   // FIXME: GLFW doesn't have this.
		cursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		cursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		cursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
		cursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
		cursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

		Gui_Init();
	}

	void ImGuiOverlay::OnUpdate()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = (float)app->deltaTime;
		io.DisplaySize = ImVec2((float)app->window->properties.width, (float)app->window->properties.height);

		Gui_NewFrame();

		UpdateGamepad();
		UpdateKey();
		UpdateMouse();

		ImGui::NewFrame();

		ShowGui();

		ImGui::Render();
		Gui_RenderDrawData(ImGui::GetDrawData());
	}

	void ImGuiOverlay::OnEvent(Event& event)
	{
		if (!event.IsOfCategory(EventType::Input))
			return;

		Handler handler(event);

		handler.Fire<MouseButtonPressedEvent>(BIND_GUI_FN(MouseButton));
		handler.Fire<MouseScrolledEvent>(BIND_GUI_FN(MouseScroll));

		handler.Fire<CharacterInputEvent>(BIND_GUI_FN(CharacterInput));
		handler.Fire<KeyPressedEvent>(BIND_GUI_FN(KeyPressed));
		handler.Fire<KeyReleasedEvent>(BIND_GUI_FN(KeyReleased));

	}

	void ImGuiOverlay::UpdateGamepad()
	{
		ImGuiIO& io = ImGui::GetIO();
		memset(io.NavInputs, 0, sizeof(io.NavInputs));
		if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
			return;

		#define MAP_BUTTON(NavInput, ButtonInput)		\
		{												\
			if (buttons_count > ButtonInput				\
				&& buttons[ButtonInput] == GLFW_PRESS)	\
				io.NavInputs[NavInput] = 1.f;			\
		}

		#define MAP_STICK(NavInput, StickInput, DeadzoneMin, DeadzoneMax)			\
		{																			\
			float v = (axes_count > StickInput) ? axes[StickInput] : DeadzoneMin;	\
			v = (v - DeadzoneMin) / (DeadzoneMax - DeadzoneMin);					\
			if (v > 1.f) v = 1.f;													\
			if (io.NavInputs[NavInput] < v)											\
				io.NavInputs[NavInput] = v;											\
		}

		int axes_count = 0, buttons_count = 0;
		const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
		const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);

		MAP_BUTTON(ImGuiNavInput_Activate, 0);     // Cross / A
		MAP_BUTTON(ImGuiNavInput_Cancel, 1);     // Circle / B
		MAP_BUTTON(ImGuiNavInput_Menu, 2);     // Square / X
		MAP_BUTTON(ImGuiNavInput_Input, 3);     // Triangle / Y
		MAP_BUTTON(ImGuiNavInput_DpadLeft, 13);    // D-Pad Left
		MAP_BUTTON(ImGuiNavInput_DpadRight, 11);    // D-Pad Right
		MAP_BUTTON(ImGuiNavInput_DpadUp, 10);    // D-Pad Up
		MAP_BUTTON(ImGuiNavInput_DpadDown, 12);    // D-Pad Down
		MAP_BUTTON(ImGuiNavInput_FocusPrev, 4);     // L1 / LB
		MAP_BUTTON(ImGuiNavInput_FocusNext, 5);     // R1 / RB
		MAP_BUTTON(ImGuiNavInput_TweakSlow, 4);     // L1 / LB
		MAP_BUTTON(ImGuiNavInput_TweakFast, 5);     // R1 / RB

		MAP_STICK(ImGuiNavInput_LStickLeft, 0, -deadzoneMin, -deadzoneMax);
		MAP_STICK(ImGuiNavInput_LStickRight, 0, deadzoneMin, deadzoneMax);
		MAP_STICK(ImGuiNavInput_LStickUp, 1, deadzoneMin, deadzoneMax);
		MAP_STICK(ImGuiNavInput_LStickDown, 1, -deadzoneMin, -deadzoneMax);

		#undef MAP_BUTTON
		#undef MAP_STICK

		if (axes_count > 0 && buttons_count > 0)
			io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
		else
			io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
	}

	void ImGuiOverlay::UpdateKey()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeyCtrl  = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT]   || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt   = io.KeysDown[GLFW_KEY_LEFT_ALT]     || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER]   || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
	}

	void ImGuiOverlay::UpdateMouse()
	{
		// Update buttons
		ImGuiIO& io = ImGui::GetIO();
		GLFWwindow* window = app->window->GetGLFWwindow();
		for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
		{
			// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
			io.MouseDown[i] = mouseJustPressed[i] || glfwGetMouseButton(window, i) != 0;
			mouseJustPressed[i] = false;
		}

		// Update mouse position
		const ImVec2 mouse_pos_backup = io.MousePos;
		io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
		if (glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0)
		{
			if (io.WantSetMousePos)
			{
				glfwSetCursorPos(window, (double)mouse_pos_backup.x, (double)mouse_pos_backup.y);
			}
			else
			{
				double mouse_x, mouse_y;
				glfwGetCursorPos(window, &mouse_x, &mouse_y);
				io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
			}
		}

		// Update mouse graphic
		if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
			return;

		ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
		if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
		{
			// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		else
		{
			// Show OS mouse cursor
			glfwSetCursor(window, cursors[imgui_cursor] ? cursors[imgui_cursor] : cursors[ImGuiMouseCursor_Arrow]);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	bool ImGuiOverlay::MouseButton(MouseButtonPressedEvent& event)
	{
		if (event.button >= 0 && event.button < IM_ARRAYSIZE(mouseJustPressed))
			mouseJustPressed[event.button] = true;
		return !ImGui::GetIO().WantCaptureMouse;
	}

	bool ImGuiOverlay::MouseScroll(MouseScrolledEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheelH += (float)event.right;
		io.MouseWheel += (float)event.up;
		return !io.WantCaptureMouse;
	}

	bool ImGuiOverlay::CharacterInput(CharacterInputEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (event.character > 0 && event.character < 0x10000)
			io.AddInputCharacter((unsigned short)event.character);
		return !io.WantCaptureKeyboard;
	}

	bool ImGuiOverlay::KeyPressed(KeyPressedEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[event.keyCode] = true;
		return !io.WantCaptureKeyboard;
	}

	bool ImGuiOverlay::KeyReleased(KeyReleasedEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[event.keyCode] = false;
		return true; // We ALWAYS want key release events to go through the rest of the application
	}

#undef BIND_GUI_FN

}
