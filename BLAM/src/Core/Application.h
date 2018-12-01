#pragma once

#include "Core.h"

namespace Blam {

	class BLAM_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	Application* CreateApplication();

}
