#pragma once

#include <Surface/Application.h>
#include <Surface/GUI/Gui.h>

using namespace Surface;
using namespace Gui;

class ClosePopup : public Gui::Window
{
public:

	ClosePopup()
		: Window("Close Surface?")
	{

	}

	bool GuiBegin() override
	{
		OpenPopup(name);
		BeginPopupModal(name, &visible);
		return false;
	}

	void Body(ImGuiWindow* window) override
	{
		Text("All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
		Separator();

		if (Button("Close", ImVec2(120, 0)))
		{
			Application::GetApp()->SendEvent(WindowClosedEvent());
		}
		SetItemDefaultFocus();

		SameLine();
		if (Button("Stay", ImVec2(120, 0)))
		{
			visible = false;
			CloseCurrentPopup();
		}
	}

	void GuiEnd() override
	{
		EndPopup();
	}
};
