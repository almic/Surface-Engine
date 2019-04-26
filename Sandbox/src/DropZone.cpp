#include <Surface.h>

class Game : public Surface::Application
{
public:
	Game()
	{

	}

	~Game()
	{

	}
};

Surface::Application* Surface::CreateApplication()
{
	return new Game();
}
