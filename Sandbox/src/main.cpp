#include "app/app.h"
#include "console/console.h"
#include "file/file.h"
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

    std::function<void(const char*)> log;

    void create_windows()
    {
        main_window = Surface::Window::create("main", {.title = "Hello World"});
        // mini_console = Surface::Window::get_console_window();
        console = Surface::Console::create("Sandbox Console", false);
    }

    void setup() override
    {
        Surface::Time::Timer timer;
        create_windows();

        log = std::bind(&Surface::Console::writeln, console, std::placeholders::_1);

        log("Setting up the application");

        auto user_data = Surface::FS::user_app_data_path();
        auto app_data = Surface::FS::sys_app_data_path();

        log("User data path:");
        log(user_data.string().c_str());
        log("App data path:");
        log(app_data.string().c_str());

        timer.log_to(log);
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
            log("Main window closed, stopping.");

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
                log("Closing console connection.");
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
    int result;

    {
        SandboxApp app;
        result = app.run();
    }

    return result;
}
