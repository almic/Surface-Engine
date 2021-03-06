#pragma once

#include "Surface/Application.h"
#include "Surface/View.h"
#include "GLRenderer.h"
#include "Gui.h"

namespace Surface {

	class DemoWindow;

	class SURF_API ImGuiOverlay : public Overlay
	{
	public:
		bool show = true;
		float deadzoneMin = 0.3f;
		float deadzoneMax = 0.9f;

		ImGuiOverlay(const std::string& name);

		virtual void ShowGui();
		virtual void Initialize();
		virtual void OnAttach() override final;
		virtual void OnUpdate() override final;
		virtual void OnEvent(Event& event) override final;

	private:
		static DemoWindow demoWindow;
		static bool initialized;
		static GLFWcursor* cursors[ImGuiMouseCursor_COUNT];
		static bool mouseJustPressed[5];
		virtual void UpdateGamepad() final;
		virtual void UpdateKey() final;
		virtual void UpdateMouse() final;

		virtual bool MouseButton(MouseButtonPressedEvent& event) final;
		virtual bool MouseScroll(MouseScrolledEvent& event) final;

		virtual bool CharacterInput(CharacterInputEvent& event) final;
		virtual bool KeyPressed(KeyPressedEvent& event) final;
		virtual bool KeyReleased(KeyReleasedEvent& event) final;
	};

}
