#pragma once

#include <Surface.h>

using namespace Surface;

class MenuLayer : public Overlay
{
public:
	MenuLayer()
		: Overlay("Menu")
	{
	}

	void OnUpdate() override
	{
		//SURF_INFO("Menu Update");
	}

	void OnEvent(Event& event) override
	{
		//SURF_INFO(event);
	}
};
