#include <BLAM.h>

class Game : public Blam::Application
{
public:
	Game()
	{
		
	}

	~Game()
	{

	}
};

Blam::Application* Blam::CreateApplication()
{
	return new Game();
}