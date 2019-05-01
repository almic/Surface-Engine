#include <Surface.h>

#include "Overlays/MenuLayer.h"

using namespace Surface;

class Game : public Application
{
public:
	Game()
	{
		View* main = new View("Main");
		
		Overlay* menu = new MenuLayer();

		main->AddOverlay(menu);

		AddView(main);
	}

	~Game()
	{

	}

};

Application* Surface::CreateApplication(int arc, char** argv)
{
	return new Game();
}
