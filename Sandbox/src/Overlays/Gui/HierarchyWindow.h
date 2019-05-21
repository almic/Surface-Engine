#pragma once

#include <Surface/GUI/Gui.h>

using namespace Surface;
using namespace Gui;

class HierarchyWindow : public Gui::Window
{
public:

	HierarchyWindow()
		: Window("HierarchyViewer", WindowFlags_Visible | WindowFlags_NoDockingUnclassed | WindowFlags_GroupWithSameType)
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
		Text("TODO: HierarchyWindow.h");
	}
};
