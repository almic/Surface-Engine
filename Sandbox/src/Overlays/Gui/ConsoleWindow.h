#pragma once

#include <Surface/GUI/Gui.h>

using namespace Surface;
using namespace Gui;

class ConsoleWindow : public Gui::Window
{
public:

	ConsoleWindow()
		: Window("ConsoleViewer", WindowFlags_Visible | WindowFlags_GroupWithSameType)
	{

	}

	bool GuiBegin() override
	{
		SetNextWindowClass(&imGuiClass);
		static ImGuiWindowFlags flags = 0
			| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoMove
			;

		Begin(name, &visible, flags);
		return true;
	}

	void Body(ImGuiWindow* window) override
	{
		Text("TODO: ConsoleWindow.h");
	}
};
