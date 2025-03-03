#include "app/app.h"
#include "window/window.h"

// #include <iostream>

class SandboxApp : public Surface::App
{
    Surface::Window* main_window;

    void setup() override
    {
        main_window = Surface::Window::create("main", {.title = "Hello World"});
    }

    void update() override
    {
        // std::cout << "Calling SandboxApp update()" << std::endl;

        main_window->update();

        if (main_window->closed || main_window->quitting)
        {
            stop(true);
        }
    };
};

int main()
{
    // std::cout << "Test" << std::endl;

    SandboxApp app;

    int result = app.run();

    // std::cout << "Stopped on tick " << app.tick_count() << std::endl;
    return result;
}
