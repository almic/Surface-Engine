#pragma once

#include <Surface.h>
#include <Surface/GUI/ImGuiOverlay.h>

// Main Menu Bar
#include "Gui/MainMenuBar.h"

// Popups
#include "Gui/ClosePopup.h"

// Floating windows
#include "Gui/GlmWindow.h"
#include "Gui/UserSettingsWindow.h"

// Main/ docked/ layout windows
#include "Gui/ConsoleWindow.h"
#include "Gui/HierarchyWindow.h"
#include "Gui/LevelWindow.h"
#include "Gui/ObjectPropertyWindow.h"

using namespace Surface;

class MenuLayer : public ImGuiOverlay
{
public:

	bool show_demo_window = false;
	char fix_layout = 1;  // Always reset layout, because dear imgui doesn't quite save the settings properly yet

	// See notes in function declarations of Gui.h
	//bool save_layout = false;

	const char* dockspace_name = "MainDockSpace";

	// Popups
	ClosePopup* closePopup;

	// Floating windows
	GlmWindow* glmWindow;
	UserSettingsWindow* userSettingsWindow;

	// Main windows
	ConsoleWindow* consoleWindow;
	HierarchyWindow* hierarchyWindow;
	LevelWindow* levelWindow;
	ObjectPropertyWindow* objectPropertyWindow;

	// Menu bar
	MainMenuBar* mainMenuBar;

	MenuLayer()
		: ImGuiOverlay("Menu")
	{
	}

	~MenuLayer()
	{
	}

	void Initialize() override
	{
		MainMenuItems items;

		// Load popups first
		items.closePopup = closePopup = new ClosePopup();

		// Load floating windows
		items.glmWindow = glmWindow = new GlmWindow();
		items.userSettingsWindow = userSettingsWindow = new UserSettingsWindow();
		items.showDemoWindow = &show_demo_window;

		// Load main windows
		items.consoleWindow = consoleWindow = new ConsoleWindow();
		items.hierarchyWindow = hierarchyWindow = new HierarchyWindow();
		items.levelWindow = levelWindow = new LevelWindow();
		items.objectPropertyWindow = objectPropertyWindow = new ObjectPropertyWindow();

		// Other stuff
		items.fixLayout = &fix_layout;

		// See notes in function declarations of Gui.h
		//items.saveLayout = &save_layout;

		// Load menu bar last, because it handles visibility for all other windows
		mainMenuBar = new MainMenuBar(items);

		LoadDefaultLayout();
	}

	void ShowGui() override
	{
		// show menu bar first
		mainMenuBar->Show();

		// create layout
		BeginDockSpace();

		// See notes in function declarations of Gui.h
		/*if (save_layout)
		{
			builder.SaveLayoutFile(builder.SaveLayout(), "hippity hoppity");
			save_layout = false;
		}*/

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

		// Build Layout
		static Builder builder = Builder();

		// We always reset the layout at the start, because dear imgui doesn't quite save the settings properly yet
		if (builder.BeginLayout(ImGui::GetID(dockspace_name), force))
		{
			ImVec2 view_size = GetCurrentWindow()->Size;
			// Default layout parameters
			float ratio_right = .17f;
			float ratio_left = (ratio_right * view_size.x) / ((1.f - ratio_right) * (view_size.x));
			float ratio_level_view = (1.f - (ratio_right * 2)) * (view_size.x) * (9.f / 16.f) / (view_size.y);

			auto[right, left] = builder.SplitRight(ratio_right);

			builder.AddRight(objectPropertyWindow, objectPropertyWindow->flags | WindowFlags_DisableCloseForce, WindowType::ASIDE_RIGHT_2);

			auto[_, middle] = builder.SplitLeft(ratio_left, left);
			builder.SplitBottom(1.f - ratio_level_view, middle);

			builder.AddLeft(hierarchyWindow, hierarchyWindow->flags | WindowFlags_DisableCloseForce, WindowType::ASIDE_RIGHT);
			builder.AddTop(levelWindow, levelWindow->flags | WindowFlags_DisableTabForce, WindowType::MAIN);
			builder.AddBottom(consoleWindow, consoleWindow->flags | WindowFlags_DisableCloseForce, WindowType::MAIN_BOTTOM);

			builder.FinishLayout();
		}

		EndDockSpace();

		// show popups/ modals
		closePopup->Show();

		// show floating windows
		glmWindow->Show();
		userSettingsWindow->Show();
		if (show_demo_window) ShowDemoWindow(&show_demo_window);

		// show main windows
		consoleWindow->Show();
		hierarchyWindow->Show();
		levelWindow->Show();
		objectPropertyWindow->Show();
	}

	void BeginDockSpace()
	{
		static bool mainLayout = true;
		static Gui::Window dockWindow;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground 
			| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoNavFocus
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoTitleBar
			;
		
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		float menubarheight = ImGui::FindWindowByName(mainMenuBar->name)->MenuBarHeight();
		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menubarheight));
		ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menubarheight));
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowClass(&dockWindow.imGuiClass);
		ImGui::Begin(dockspace_name, &mainLayout, window_flags);
		ImGui::PopStyleVar(3);
	}

	void EndDockSpace()
	{
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode
		      | ImGuiDockNodeFlags_NoDockingInCentralNode
		      ;

		ImGuiID dockspace_id = ImGui::GetID(dockspace_name);
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		ImGui::End();
	}

	void LoadDefaultLayout()
	{
		// TODO: load layout from file system, keep hardcode as backup
	}

};
