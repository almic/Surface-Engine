#pragma once

#include <Surface/GUI/Gui.h>

using namespace Surface;
using namespace Gui;

class ObjectPropertyWindow : public Gui::Window
{
public:

	ObjectPropertyWindow()
		: Window("ObjectPropertyViewer", WindowFlags_Visible | WindowFlags_NoDockingUnclassed)
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
		Text("TODO: ObjectPropertyWindow.h");
	}
};
