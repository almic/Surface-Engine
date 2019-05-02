#include <Surface.h>

#include "Overlays/MenuLayer.h"

using namespace Surface;

class Game : public Application
{
public:

	double fpsLimiter = 0;
	int fpsCounter = 0;

	Game()
	{
		window->SetTargetFPS(60);
		//window->SetVSync(false);

		View* main = new View("Main");
		
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
		if (fpsLimiter >= 0.1)
		{
			SURF_TRACE("FPS: {0}", (fpsCounter / fpsLimiter));
			fpsCounter = 0;
			fpsLimiter = 0;
		}
	}

};

Application* Surface::CreateApplication(int arc, char** argv)
{
	return new Game();
}
