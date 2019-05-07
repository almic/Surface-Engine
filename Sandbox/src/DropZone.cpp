#include <Surface.h>

#include "Overlays/MenuLayer.h"

namespace Surface {

	class Game : public Application
	{
	public:

		double fpsLimiter = 0;
		int fpsCounter = 0;

		Game() : Application(WindowProperties(
			"Surface", // Title
			1920,      // Width
			1080,      // Height
			0,         // x
			0,         // y
			false,     // Leaves position unset
			false,     // Disables OpenGL vsync
			60,        // Framerate "capped" to 60FPS
			WindowForm::WINDOWED_FULLSCREEN
		))
		{
			View* main = new View(this, "Main");

			Overlay* menu = new MenuLayer();

			main->AddOverlay(menu);

			AddView(main);
		}

		~Game()
		{
		}

		void OnTick(const double& deltaTime) override
		{
			fpsLimiter += deltaTime;
			fpsCounter++;
			if (fpsLimiter >= 1)
			{
				SURF_TRACE("FPS: {0}", (fpsCounter / fpsLimiter));
				fpsCounter = 0;
				fpsLimiter = 0;
			}
		}

	};

	Application* CreateApplication(int arc, char** argv)
	{
		return new Game();
	}

}
