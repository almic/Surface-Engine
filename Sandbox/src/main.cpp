#include "app/app.h"
#include "console/console.h"
#include "window/window.h"

class SandboxApp : public Surface::App
{
    Surface::Window* main_window = nullptr;
    Surface::Console* console = nullptr;

    void setup() override
    {
        main_window = Surface::Window::create("main", {.title = "Hello World"});

        console = Surface::Console::create("Sanbox Console", false);

        console->writeln("Setting up the application");
    }

    void update() override
    {
        // Send buffered text
        if (console->is_buffered())
        {
            console->flush();
        }

        if (tick_count() == 1000000)
        {
            console->writeln("1 million ticks!");
        }

        main_window->update();

        if (main_window->closed || main_window->quitting)
        {
            console->writeln("Main window closed, stopping.");
            if (console->is_open())
            {
                console->writeln("Closing console connection.");
                console->end();
            }
            delete console;
            stop(true);
        }
    };
};

int main()
{
    SandboxApp app;

    int result = app.run();

    return result;
}
