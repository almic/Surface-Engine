#include "app/app.h"
#include "console/console.h"
#include "time/time.h"
#include "window/window.h"

#include <functional>
#include <iostream>
#include <thread>

#undef NULL

class SandboxApp : public Surface::App
{
    Surface::Window* main_window = nullptr;
    Surface::Window* mini_console = nullptr;
    Surface::Console* console = nullptr;

    void setup() override
    {
        Surface::Time::Timer timer;
        main_window = Surface::Window::create("main", {.title = "Hello World"});
        // mini_console = Surface::Window::get_console_window();
        console = Surface::Console::create("Sanbox Console", false);

        console->writeln("Setting up the application");
        timer.log_to(std::bind(&Surface::Console::writeln, console, std::placeholders::_1));
    }

    void update() override
    {
        // Send buffered text
        if (console->is_buffered())
        {
            console->flush();
        }

        main_window->update();

        if (main_window->closed || main_window->quitting)
        {
            console->writeln("Main window closed, stopping.");

            stop(true);
        }
    };

    void teardown() override
    {
        if (main_window)
        {
            delete main_window;
        }

        if (console)
        {
            if (console->is_open())
            {
                console->writeln("Closing console connection.");
                console->end();

                // wait for a little while until the buffer is empty
                uint8_t max = 50;
                while (max && console->is_buffered())
                {
                    --max;
                    if (!console->flush())
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        continue;
                    }
                    break;
                }
            }
            delete console;
        }

        if (mini_console)
        {
            delete mini_console;
        }
    }
};

int main()
{
    SandboxApp app;

    int result = app.run();

    return result;
}
