#pragma once

#include <Surface/GUI/Gui.h>

// Popups
#include "ClosePopup.h"

// Floating windows
#include "GlmWindow.h"
#include "UserSettingsWindow.h"

// Main/ docked/ layout windows
#include "ConsoleWindow.h"
#include "HierarchyWindow.h"
#include "LevelWindow.h"
#include "ObjectPropertyWindow.h"

using namespace Surface;
using namespace Gui;

struct MainMenuItems
{
	// Popups
	ClosePopup* closePopup;

	// Floating Windows
	GlmWindow* glmWindow;
	UserSettingsWindow* userSettingsWindow;
	bool* showDemoWindow;

	// Main Windows
	ConsoleWindow* consoleWindow;
	HierarchyWindow* hierarchyWindow;
	LevelWindow* levelWindow;
	ObjectPropertyWindow* objectPropertyWindow;

	// Others
	char* fixLayout;
	bool* saveLayout;
};

class MainMenuBar : public Gui::Window
{
public:

	MainMenuItems items;

	MainMenuBar(MainMenuItems items)
		: items(items),
		Window("##MainMenuBar", WindowFlags_Visible | WindowFlags_NoDockingUnclassed)
	{
		
	}

	bool GuiBegin() override
	{
		BeginMainMenuBar();
		return false;
	}

	void Body(ImGuiWindow* window) override
	{
		if (BeginMenu("File"))
		{
			// TODO: Basic file things
			MenuItem("Close", NULL, items.closePopup->visible);
			
			// EndMenu(); fuck you WinUser.h
			ImGui::EndMenu();
		}

		if (BeginMenu("Edit"))
		{
			// TODO: Editing functions, often context specific
			// EndMenu();
			ImGui::EndMenu();
		}

		if (BeginMenu("View"))
		{
			// TODO: Displaying windows or other viewport-related functions
			TextDisabled("Windows");
			MenuItem("User Settings", NULL, &items.userSettingsWindow->visible);
			MenuItem("ImGui Demo", NULL, items.showDemoWindow);
			MenuItem("GLM Demo", NULL, &items.glmWindow->visible);

			Separator();
			TextDisabled("Functions");
			if (MenuItem("Reset Layout"))
			{
				*items.fixLayout = 2;
			}

			// See notes in function declarations of Gui.h
			/*if (MenuItem("Save Layout"))
			{
				*items.saveLayout = true;
			}*/

			// EndMenu();
			ImGui::EndMenu();
		}
	}

	void GuiEnd() override
	{
		ImGui::EndMainMenuBar();
	}
};
