#pragma once

#include <Surface.h>
#include <Surface/GUI/ImGuiOverlay.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <glm/geometric.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

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
			if (ImGui::TreeNode("Views"))
			{
				ImGui::NewLine();
				ImGui::Text("Use the inputs and buttons to create matrices");
				ImGui::Spacing();

				static float m1_00, m1_01, m1_02, m1_03,
				             m1_10, m1_11, m1_12, m1_13,
				             m1_20, m1_21, m1_22, m1_23,
				             m1_30, m1_31, m1_32, m1_33;

				static glm::mat4 m1 = {
					m1_00, m1_01, m1_02, m1_03,
					m1_10, m1_11, m1_12, m1_13,
					m1_20, m1_21, m1_22, m1_23,
					m1_30, m1_31, m1_32, m1_33
				};

				static glm::mat4 m2;

				ImGui::Text("Frustum");
				static float fl, fr, fb, ft, fn, ff;
				ImGui::InputFloat("Left", &fl, .1f, 1.f);
				ImGui::InputFloat("Right", &fr, .1f, 1.f);
				ImGui::InputFloat("Top", &ft, .1f, 1.f);
				ImGui::InputFloat("Bottom", &fb, .1f, 1.f);
				ImGui::InputFloat("Near", &fn, .1f, 1.f);
				ImGui::InputFloat("Far", &ff, .1f, 1.f);
				if (ImGui::Button("Create Frustum"))
				{
					m2 = glm::frustum(fl, fr, fb, ft, fn, ff);
				}
				ImGui::NewLine();

				ImGui::Text("Perspective");
				static float pfov, pasp_x, pasp_y, pn, pf;
				ImGui::InputFloat("FOV", &pfov, .1f, 1.f);
				ImGui::InputFloat("Aspect Ratio Width", &pasp_x, .1f, 1.f);
				ImGui::InputFloat("Aspect Ratio Height", &pasp_y, .1f, 1.f);
				ImGui::InputFloat("Near ", &pn, .1f, 1.f);
				ImGui::InputFloat("Far (0 = infinite)", &pf, .1f, 1.f);
				if (ImGui::Button("Create Perspective"))
				{
					if (pf == 0)
					{
						m2 = glm::infinitePerspective(pfov, pasp_x / pasp_y, pn);
					}
					else
					{
						m2 = glm::perspective(pfov, pasp_x / pasp_y, pn, pf);
					}
				}
				ImGui::NewLine();

				ImGui::Text("Orthogonal");
				static float ol, or, ob, ot;
				ImGui::InputFloat("Left ", &ol, .1f, 1.f);
				ImGui::InputFloat("Right ", &or, .1f, 1.f);
				ImGui::InputFloat("Top ", &ot, .1f, 1.f);
				ImGui::InputFloat("Bottom ", &ob, .1f, 1.f);
				if (ImGui::Button("Create Orthogonal"))
				{
					m2 = glm::ortho(ol, or, ob, ot);
				}
				ImGui::NewLine();

				ImGui::Text("Orthogonal (Clipped)");
				static float ocl, ocr, ocb, oct, ocn, ocf;
				ImGui::InputFloat("Left  ", &ocl, .1f, 1.f);
				ImGui::InputFloat("Right  ", &ocr, .1f, 1.f);
				ImGui::InputFloat("Top  ", &oct, .1f, 1.f);
				ImGui::InputFloat("Bottom  ", &ocb, .1f, 1.f);
				ImGui::InputFloat("Near  ", &ocn, .1f, 1.f);
				ImGui::InputFloat("Far ", &ocf, .1f, 1.f);
				if (ImGui::Button("Create Orthogonal (Clipped)"))
				{
					m2 = glm::ortho(ocl, ocr, ocb, oct, ocn, ocf);
				}
				ImGui::NewLine();

				ImGui::Spacing();
				ImGui::Text("Result:\n[ %.3f , %.3f , %.3f , %.3f ]\n[ %.3f , %.3f , %.3f , %.3f ]\n[ %.3f , %.3f , %.3f , %.3f ]\n[ %.3f , %.3f , %.3f , %.3f ]",
					m2[0][0], m2[0][1], m2[0][2], m2[0][3],
					m2[1][0], m2[1][1], m2[1][2], m2[1][3],
					m2[2][0], m2[2][1], m2[2][2], m2[2][3],
					m2[3][0], m2[3][1], m2[3][2], m2[3][3]);
				ImGui::NewLine();

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Transformation"))
			{
				ImGui::NewLine();
				ImGui::Text("Create and transform matrix with a vector");
				ImGui::Spacing();
				
				static float m3_00 = 1.f, m3_01 = 1.f, m3_02 = 1.f, m3_03 = 1.f,
					         m3_10 = 1.f, m3_11 = 1.f, m3_12 = 1.f, m3_13 = 1.f,
					         m3_20 = 1.f, m3_21 = 1.f, m3_22 = 1.f, m3_23 = 1.f,
					         m3_30 = 1.f, m3_31 = 1.f, m3_32 = 1.f, m3_33 = 1.f;

				static glm::mat4 m3 = {
					m3_00, m3_01, m3_02, m3_03,
					m3_10, m3_11, m3_12, m3_13,
					m3_20, m3_21, m3_22, m3_23,
					m3_30, m3_31, m3_32, m3_33
				};

				ImGui::Text("Matrix to tranform:");
				ImGui::InputFloat4("##R1", (float*)&m3[0]);
				ImGui::InputFloat4("##R2", (float*)&m3[1]);
				ImGui::InputFloat4("##R3", (float*)&m3[2]);
				ImGui::InputFloat4("##R4", (float*)&m3[3]);
				ImGui::NewLine();

				static float mt_x, mt_y, mt_z, mt_a;
				static glm::vec3 mtv = { mt_x, mt_y, mt_z };
				static glm::mat4 m4;

				ImGui::Text("Parameters:");
				ImGui::DragFloat3("Vector", (float*)&mtv, .01f);
				ImGui::DragFloat("Angle (rotate only)", &mt_a);

				ImGui::Spacing();
				if (ImGui::Button("Rotate"))
					m4 = glm::rotate(m3, glm::radians(mt_a), mtv);

				ImGui::SameLine();
				if (ImGui::Button("Scale"))
					m4 = glm::scale(m3, mtv);

				ImGui::SameLine();
				if (ImGui::Button("Translate"))
					m4 = glm::translate(m3, mtv);

				ImGui::Spacing();
				ImGui::Text("Result:\n[ %.3f , %.3f , %.3f , %.3f ]\n[ %.3f , %.3f , %.3f , %.3f ]\n[ %.3f , %.3f , %.3f , %.3f ]\n[ %.3f , %.3f , %.3f , %.3f ]",
					m4[0][0], m4[0][1], m4[0][2], m4[0][3],
					m4[1][0], m4[1][1], m4[1][2], m4[1][3],
					m4[2][0], m4[2][1], m4[2][2], m4[2][3],
					m4[3][0], m4[3][1], m4[3][2], m4[3][3]);
				ImGui::NewLine();

				ImGui::TreePop();
			}
		}

		ImGui::End();
	}

};
