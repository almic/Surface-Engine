#pragma once

#include <Surface.h>
#include <Surface/GUI/ImGuiOverlay.h>

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

using namespace Surface;

class MenuLayer : public ImGuiOverlay
{
public:

	bool show_demo_window = false;
	bool show_close_confirm = false;
	bool show_glm_window = false;

	MenuLayer()
		: ImGuiOverlay("Menu")
	{
	}

	~MenuLayer()
	{
	}

	void ShowGui() override
	{
		ShowMainMenuBar();

		if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
		if (show_close_confirm) ShowCloseConfirm();
		if (show_glm_window) ShowGLMWindow();
	}

	void ShowMainMenuBar()
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
				ImGui::MenuItem("GLM Demo", NULL, &show_glm_window);

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void ShowCloseConfirm()
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

	void ShowGLMWindow()
	{
		if (!ImGui::Begin("GLM Demo", &show_glm_window))
		{
			ImGui::End();
			return;
		}

		ImGui::Text("Some GLM (maths) demo thing");

		if (ImGui::CollapsingHeader("Vector"))
		{
			if (ImGui::TreeNode("Cross Product"))
			{
				static float x1 = 3.f, y1 = -3.f, z1 = 1.f,
					         x2 = 4.f, y2 =  9.f, z2 = 2.f;

				static glm::vec3 vec_a = { x1, y1, z1 };
				static glm::vec3 vec_b = { x2, y2, z2 };
				static glm::vec3 vec_c;

				ImGui::Text("Vector A:");
				ImGui::DragFloat3("##Vector A", (float*)&vec_a, .1f);

				ImGui::Text("Vector B:");
				ImGui::DragFloat3("##Vector B", (float*)&vec_b, .1f);

				if (ImGui::Button("Calculate"))
				{
					vec_c = glm::cross(vec_a, vec_b);
				}

				ImGui::Text("Result: { %.3f , %.3f , %.3f }", vec_c.x, vec_c.y, vec_c.z);
				
				ImGui::TreePop();
			}
		}

		if (ImGui::CollapsingHeader("Matrix"))
		{

		}

		if (ImGui::CollapsingHeader("Quaternion"))
		{

		}

		//if (ImGui::CollapsingHeader(""))

		ImGui::End();
	}

};
