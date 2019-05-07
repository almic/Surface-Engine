#pragma once

#include <Surface.h>
#include <Surface/GUI/ImGuiOverlay.h>

using namespace Surface;

class MenuLayer : public ImGuiOverlay
{
public:

	bool show_demo_window = false;
	bool show_close_confirm = false;

	MenuLayer()
		: ImGuiOverlay("Menu")
	{
	}

	~MenuLayer()
	{
	}

	void ShowGui() override
	{
		ShowMainMenu();
	}

	void ShowMainMenu()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// TODO: Basic file things
				ImGui::MenuItem("Close", NULL, &show_close_confirm);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				// TODO: Edit functions
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("ImGui Demo", NULL, &show_demo_window);

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
		if (show_close_confirm)
		{
			ImGui::OpenPopup("Close Surface?");

			if (ImGui::BeginPopupModal("Close Surface?", &show_close_confirm, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
				ImGui::Separator();

				if (ImGui::Button("Close", ImVec2(120, 0)))
				{
					app->SendEvent(WindowClosedEvent());
				}
				ImGui::SetItemDefaultFocus();

				ImGui::SameLine();
				if (ImGui::Button("Stay", ImVec2(120, 0)))
				{
					show_close_confirm = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}
	}
};
