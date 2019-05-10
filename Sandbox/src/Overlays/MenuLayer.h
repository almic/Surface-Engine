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

		if (show_close_confirm) ShowCloseConfirm();
		if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
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
				ImGui::NewLine();
				static float v1_x = 3.f, v1_y = -3.f, v1_z = 1.f,
					         v2_x = 4.f, v2_y =  9.f, v2_z = 2.f;

				static glm::vec3 vec_1 = { v1_x, v1_y, v1_z };
				static glm::vec3 vec_2 = { v2_x, v2_y, v2_z };
				static glm::vec3 vec_3;

				static bool changed = true;

				ImGui::Text("Vector A:");
				changed = ImGui::DragFloat3("##Vector A", (float*)&vec_1, .1f) || changed;

				ImGui::Text("Vector B:");
				changed = ImGui::DragFloat3("##Vector B", (float*)&vec_2, .1f) || changed;

				if (changed)
				{
					vec_3 = glm::cross(vec_1, vec_2);
					changed = false;
				}

				ImGui::Spacing();
				ImGui::Text("Result: { %.3f , %.3f , %.3f }", vec_3.x, vec_3.y, vec_3.z);
				ImGui::NewLine();

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Distance"))
			{
				ImGui::NewLine();
				static float p1_x = 2.f, p1_y = 1.5f, p1_z = -.25f,
					         p2_x = -1.f, p2_y = 5.f, p2_z = 2.f;

				static glm::vec3 p1 = { p1_x, p1_y, p1_z };
				static glm::vec3 p2 = { p2_x, p2_y, p2_z };
				static float distance;

				static bool changed = true;

				ImGui::Text("Point A:");
				changed = ImGui::DragFloat3("##Point A", (float*)&p1, .1f) || changed;

				ImGui::Text("Vector B:");
				changed = ImGui::DragFloat3("##Point B", (float*)&p2, .1f) || changed;

				if (changed)
				{
					distance = glm::distance(p1, p2);
					changed = false;
				}

				ImGui::Spacing();
				ImGui::Text("Distance: %.3f", distance);
				ImGui::NewLine();

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Dot Product"))
			{
				ImGui::NewLine();
				static float v4_x = -12.f, v4_y = 16.f, v4_z = -4.f,
					         v5_x =  12.f, v5_y =  9.f, v5_z =  0.f;

				static glm::vec3 vec_4 = { v4_x, v4_y, v4_z };
				static glm::vec3 vec_5 = { v5_x, v5_y, v5_z };
				static float dot;
				
				static bool changed = true;

				ImGui::Text("Vector A:");
				changed = ImGui::DragFloat3("##Vector A", (float*)&vec_4, .1f) || changed;

				ImGui::Text("Vector B:");
				changed = ImGui::DragFloat3("##Vector B", (float*)&vec_5, .1f) || changed;

				if (changed)
				{
					dot = glm::dot(vec_4, vec_5);
					changed = false;
				}

				ImGui::Spacing();
				ImGui::Text("Result: %.3f", dot);
				ImGui::NewLine();

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Length"))
			{
				ImGui::NewLine();

				static float v6_x = 3.f, v6_y = -.1f, v6_z = 1.f;

				static glm::vec3 vec_6 = { v6_x, v6_y, v6_z };
				static float length = glm::length(vec_6);

				ImGui::Text("Vector");
				if (ImGui::DragFloat3("##Vector", (float*)&vec_6, .1f))
				{
					length = glm::length(vec_6);
				}

				ImGui::Spacing();
				ImGui::Text("Length: %.3f", length);
				ImGui::NewLine();
				
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Normalize"))
			{
				ImGui::NewLine();

				static float v7_x = 1.f, v7_y = .8f, v7_z = -.5f;

				static glm::vec3 vec_7 = { v7_x, v7_y, v7_z };
				static glm::vec3 vec_8 = glm::normalize(vec_7);

				ImGui::Text("Vector");
				if (ImGui::DragFloat3("##Vector", (float*)&vec_7, .01f))
				{
					vec_8 = glm::normalize(vec_7);
				}

				ImGui::Spacing();
				ImGui::Text("Normalized: { %.4f , %.4f , %.4f }", vec_8.x, vec_8.y, vec_8.z);
				ImGui::NewLine();

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
