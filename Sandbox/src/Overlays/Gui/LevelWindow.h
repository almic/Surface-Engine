#pragma once

#include <Surface/GUI/Gui.h>

using namespace Surface;
using namespace Gui;

class LevelWindow : public Gui::Window
{
public:

	LevelWindow()
		: Window("LevelViewer", WindowFlags_Visible | WindowFlags_NoDockingUnclassed)
	{
		
	}

	bool GuiBegin() override
	{
		SetNextWindowClass(&imGuiClass);
		static ImGuiWindowFlags flags = 0
			| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoDecoration
			| ImGuiWindowFlags_NoMove
			;
		Begin(name, &visible, flags);
		return true;
	}

	void Body(ImGuiWindow* window) override
	{
		Text("TODO: LevelWindow.h");
		Text("%.0f , %.0f", window->Size.x, window->Size.y);
	}

};
