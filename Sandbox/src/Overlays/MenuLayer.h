#pragma once

#include <Surface.h>
#include <Surface/GUI/ImGuiOverlay.h>

using namespace Surface;

class MenuLayer : public ImGuiOverlay
{
public:
	MenuLayer()
		: ImGuiOverlay("Menu")
	{
	}

	~MenuLayer()
	{
	}
};
