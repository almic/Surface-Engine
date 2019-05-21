#pragma once

#include <Surface/GUI/Gui.h>

using namespace Surface;
using namespace Gui;

// This window actually uses three different windows, just to make viewing easier.
// One window to hold a list of settings, and another to display those options.
// The third and final window just docks them together.

class UserSettingsWindow;

class UserSettingsList : public Gui::Window
{
	friend UserSettingsWindow;
private:
	// Private constructor to prevent accidental use
	UserSettingsList()
		: Window("UserSettingsList", WindowFlags_Visible | WindowFlags_NoDockingUnclassed)
	{

	}

public:

	bool GuiBegin() override
	{
		SetNextWindowClass(&imGuiClass);

		static ImGuiWindowFlags flags = 0
			| ImGuiWindowFlags_AlwaysVerticalScrollbar // Helps break up the two windows
			//| ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoDecoration
			//| ImGuiWindowFlags_NoMove
			//| ImGuiWindowFlags_NoSavedSettings // This breaks docking
			;
		Begin(name, NULL, flags); // Always show this inside the settings window
		return true;
	}

	void Body(ImGuiWindow* window) override
	{
		Text("TODO: UserSettingsWindow.h List");
	}

	void GuiEnd() override
	{
		End();
	}
};

class UserSettingsViewer : public Gui::Window
{
	friend UserSettingsWindow;
private:
	// Private constructor to prevent accidental use
	UserSettingsViewer()
		: Window("UserSettingsViewer", WindowFlags_Visible | WindowFlags_NoDockingUnclassed)
	{

	}

public:

	bool GuiBegin() override
	{
		SetNextWindowClass(&imGuiClass);

		static ImGuiWindowFlags flags = 0
			//| ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoDecoration
			//| ImGuiWindowFlags_NoMove
			//| ImGuiWindowFlags_NoSavedSettings // This breaks docking
			;
		Begin(name, NULL, flags); // Always show this inside the settings window
		return true;
	}

	void Body(ImGuiWindow* window) override
	{
		Text("TODO: UserSettingsWindow.h Viewer");
	}

	void GuiEnd() override
	{
		End();
	}
};

class UserSettingsWindow : public Gui::Window
{
public:

	UserSettingsList* list;
	UserSettingsViewer* viewer;
	char fix_layout = 1; // Always reset layout, because dear imgui doesn't quite save the settings properly yet

	UserSettingsWindow()
		: Window("User Settings", WindowFlags_NoDockingUnclassed)
	{
		list = new UserSettingsList();
		viewer = new UserSettingsViewer();

		// This stops docking into the entire window. Actually a bug that this works!
		list->imGuiClass.ClassId = GetUniqueClass();
		viewer->imGuiClass.ClassId = GetUniqueClass();
		
		// Since the above is a bug, this should future proof the thing.
		// In fact, this line actually has no current affect on docking, which is the bug!
		imGuiClass.ClassId = GetUniqueClass();
	}

	bool GuiBegin() override
	{
		SetNextWindowClass(&imGuiClass);
		static ImGuiWindowFlags flags = 0
			// | ImGuiWindowFlags_AlwaysAutoResize
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoDocking
			// | ImGuiWindowFlags_NoResize // TODO: Should probably use custom min-max window sizes instead
			;

		SetNextWindowSize(ImVec2(600, 300), ImGuiCond_Once);
		PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		Begin(name, &visible, flags);
		PopStyleVar(1);
		return true;
	}

	void Body(ImGuiWindow* window) override
	{

		bool force = false;
		if (fix_layout == 1)
		{
			fix_layout = 2;
		}
		else if (fix_layout == 2)
		{
			fix_layout = 0;
			force = true;
		}
		else
		{
			fix_layout = 0;
		}

		ImGuiID id = GetID(name);

		static Builder builder = Builder();

		if (builder.BeginLayout(id, force))
		{
			builder.SplitLeft(.25f);

			builder.AddLeft(list, list->flags | WindowFlags_DisableTabForce);
			builder.AddRight(viewer, viewer->flags | WindowFlags_DisableTabForce);

			builder.FinishLayout();
		}
		
		ImGuiDockNodeFlags dockspace_flags = 0
			| ImGuiDockNodeFlags_PassthruCentralNode
			;

		DockSpace(id, ImVec2(0.f, 0.f), dockspace_flags);
	}

	void GuiEnd() override
	{
		End();

		// Show internal windows
		list->Show();
		viewer->Show();
	}
};
