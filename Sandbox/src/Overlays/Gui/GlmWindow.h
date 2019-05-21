#pragma once

#include <Surface/GUI/Gui.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <glm/geometric.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

using namespace Surface;
using namespace Gui;

class GlmWindow : public Gui::Window
{
public:

	GlmWindow()
		: Window("GLM Demo")
	{

	}

	void Body(ImGuiWindow* window) override
	{
		Text("Some GLM (maths) demo thing");

		if (CollapsingHeader("Vector"))
		{
			if (TreeNode("Cross Product"))
			{
				NewLine();
				static float v1_x = 3.f, v1_y = -3.f, v1_z = 1.f,
					v2_x = 4.f, v2_y = 9.f, v2_z = 2.f;

				static glm::vec3 vec_1 = { v1_x, v1_y, v1_z };
				static glm::vec3 vec_2 = { v2_x, v2_y, v2_z };
				static glm::vec3 vec_3;

				static bool changed = true;

				Text("Vector A:");
				changed = DragFloat3("##Vector A", (float*)&vec_1, .1f) || changed;

				Text("Vector B:");
				changed = DragFloat3("##Vector B", (float*)&vec_2, .1f) || changed;

				if (changed)
				{
					vec_3 = glm::cross(vec_1, vec_2);
					changed = false;
				}

				Spacing();
				Text("Result: { %.3f , %.3f , %.3f }", vec_3.x, vec_3.y, vec_3.z);
				NewLine();

				TreePop();
			}

			if (TreeNode("Distance"))
			{
				NewLine();
				static float p1_x = 2.f, p1_y = 1.5f, p1_z = -.25f,
					p2_x = -1.f, p2_y = 5.f, p2_z = 2.f;

				static glm::vec3 p1 = { p1_x, p1_y, p1_z };
				static glm::vec3 p2 = { p2_x, p2_y, p2_z };
				static float distance;

				static bool changed = true;

				Text("Point A:");
				changed = DragFloat3("##Point A", (float*)&p1, .1f) || changed;

				Text("Vector B:");
				changed = DragFloat3("##Point B", (float*)&p2, .1f) || changed;

				if (changed)
				{
					distance = glm::distance(p1, p2);
					changed = false;
				}

				Spacing();
				Text("Distance: %.3f", distance);
				NewLine();

				TreePop();
			}

			if (TreeNode("Dot Product"))
			{
				NewLine();
				static float v4_x = -12.f, v4_y = 16.f, v4_z = -4.f,
					v5_x = 12.f, v5_y = 9.f, v5_z = 0.f;

				static glm::vec3 vec_4 = { v4_x, v4_y, v4_z };
				static glm::vec3 vec_5 = { v5_x, v5_y, v5_z };
				static float dot;

				static bool changed = true;

				Text("Vector A:");
				changed = DragFloat3("##Vector A", (float*)&vec_4, .1f) || changed;

				Text("Vector B:");
				changed = DragFloat3("##Vector B", (float*)&vec_5, .1f) || changed;

				if (changed)
				{
					dot = glm::dot(vec_4, vec_5);
					changed = false;
				}

				Spacing();
				Text("Result: %.3f", dot);
				NewLine();

				TreePop();
			}

			if (TreeNode("Length"))
			{
				NewLine();

				static float v6_x = 3.f, v6_y = -.1f, v6_z = 1.f;

				static glm::vec3 vec_6 = { v6_x, v6_y, v6_z };
				static float length = glm::length(vec_6);

				Text("Vector");
				if (DragFloat3("##Vector", (float*)&vec_6, .1f))
				{
					length = glm::length(vec_6);
				}

				Spacing();
				Text("Length: %.3f", length);
				NewLine();

				TreePop();
			}

			if (TreeNode("Normalize"))
			{
				NewLine();

				static float v7_x = 1.f, v7_y = .8f, v7_z = -.5f;

				static glm::vec3 vec_7 = { v7_x, v7_y, v7_z };
				static glm::vec3 vec_8 = glm::normalize(vec_7);

				Text("Vector");
				if (DragFloat3("##Vector", (float*)&vec_7, .01f))
				{
					vec_8 = glm::normalize(vec_7);
				}

				Spacing();
				Text("Normalized: { %.4f , %.4f , %.4f }", vec_8.x, vec_8.y, vec_8.z);
				NewLine();

				TreePop();
			}
		}

		if (CollapsingHeader("Matrix"))
		{
			if (TreeNode("Views"))
			{
				NewLine();
				Text("Use the inputs and buttons to create matrices");
				Spacing();

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

				Text("Frustum");
				static float fl, fr, fb, ft, fn, ff;
				InputFloat("Left###Leftfrust", &fl, .1f, 1.f);
				InputFloat("Right###Rightfrust", &fr, .1f, 1.f);
				InputFloat("Top###Topfrust", &ft, .1f, 1.f);
				InputFloat("Bottom###Bottomfrust", &fb, .1f, 1.f);
				InputFloat("Near###Nearfrust", &fn, .1f, 1.f);
				InputFloat("Far###Farfrust", &ff, .1f, 1.f);
				if (Button("Create Frustum"))
				{
					m2 = glm::frustum(fl, fr, fb, ft, fn, ff);
				}
				NewLine();

				Text("Perspective");
				static float pfov, pasp_x, pasp_y, pn, pf;
				InputFloat("FOV###FOVpersp", &pfov, .1f, 1.f);
				InputFloat("Aspect Ratio Width###aspwpersp", &pasp_x, .1f, 1.f);
				InputFloat("Aspect Ratio Height###asphpersp", &pasp_y, .1f, 1.f);
				InputFloat("Near###Nearpersp", &pn, .1f, 1.f);
				InputFloat("Far (0 = infinite)###Farpersp", &pf, .1f, 1.f);
				if (Button("Create Perspective"))
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
				NewLine();

				Text("Orthogonal");
				static float ol, or , ob, ot;
				InputFloat("Left###Leftortho", &ol, .1f, 1.f);
				InputFloat("Right###Rightortho", &or , .1f, 1.f);
				InputFloat("Top###Toportho", &ot, .1f, 1.f);
				InputFloat("Bottom###Bottomortho", &ob, .1f, 1.f);
				if (Button("Create Orthogonal"))
				{
					m2 = glm::ortho(ol, or , ob, ot);
				}
				NewLine();

				Text("Orthogonal (Clipped)");
				static float ocl, ocr, ocb, oct, ocn, ocf;
				InputFloat("Left###Leftorthocl", &ocl, .1f, 1.f);
				InputFloat("Right###Rightorthocl", &ocr, .1f, 1.f);
				InputFloat("Top###Toporthocl", &oct, .1f, 1.f);
				InputFloat("Bottom###Bottomorthocl", &ocb, .1f, 1.f);
				InputFloat("Near###Nearorthocl", &ocn, .1f, 1.f);
				InputFloat("Far###Farorthocl", &ocf, .1f, 1.f);
				if (Button("Create Orthogonal (Clipped)"))
				{
					m2 = glm::ortho(ocl, ocr, ocb, oct, ocn, ocf);
				}
				NewLine();

				Spacing();
				Text("Result:\n[ %.3f , %.3f , %.3f , %.3f ]\n[ %.3f , %.3f , %.3f , %.3f ]\n[ %.3f , %.3f , %.3f , %.3f ]\n[ %.3f , %.3f , %.3f , %.3f ]",
					m2[0][0], m2[0][1], m2[0][2], m2[0][3],
					m2[1][0], m2[1][1], m2[1][2], m2[1][3],
					m2[2][0], m2[2][1], m2[2][2], m2[2][3],
					m2[3][0], m2[3][1], m2[3][2], m2[3][3]);
				NewLine();

				TreePop();
			}

			if (TreeNode("Transformation"))
			{
				NewLine();
				Text("Create and transform matrix with a vector");
				Spacing();

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

				Text("Matrix to tranform:");
				InputFloat4("##R1", (float*)&m3[0]);
				InputFloat4("##R2", (float*)&m3[1]);
				InputFloat4("##R3", (float*)&m3[2]);
				InputFloat4("##R4", (float*)&m3[3]);
				NewLine();

				static float mt_x, mt_y, mt_z, mt_a;
				static glm::vec3 mtv = { mt_x, mt_y, mt_z };
				static glm::mat4 m4;

				Text("Parameters:");
				DragFloat3("Vector", (float*)&mtv, .01f);
				DragFloat("Angle (rotate only)", &mt_a);

				Spacing();
				if (Button("Rotate"))
					m4 = glm::rotate(m3, glm::radians(mt_a), mtv);

				SameLine();
				if (Button("Scale"))
					m4 = glm::scale(m3, mtv);

				SameLine();
				if (Button("Translate"))
					m4 = glm::translate(m3, mtv);

				Spacing();
				Text("Result:\n[ %.3f , %.3f , %.3f , %.3f ]\n[ %.3f , %.3f , %.3f , %.3f ]\n[ %.3f , %.3f , %.3f , %.3f ]\n[ %.3f , %.3f , %.3f , %.3f ]",
					m4[0][0], m4[0][1], m4[0][2], m4[0][3],
					m4[1][0], m4[1][1], m4[1][2], m4[1][3],
					m4[2][0], m4[2][1], m4[2][2], m4[2][3],
					m4[3][0], m4[3][1], m4[3][2], m4[3][3]);
				NewLine();

				TreePop();
			}
		}
	}
};
