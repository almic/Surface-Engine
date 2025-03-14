#include "app/app.h"
#include "console/console.h"
#include "window/window.h"

#include "json/json.h"
#include <iostream>

#undef NULL

namespace JSON = Surface::JSON;

class SandboxApp : public Surface::App
{
    Surface::Window* main_window = nullptr;
    Surface::Window* mini_console = nullptr;
    Surface::Console* console = nullptr;

    void setup() override
    {
        main_window = Surface::Window::create("main", {.title = "Hello World"});
        // mini_console = Surface::Window::get_console_window();
        console = Surface::Console::create("Sanbox Console", false);

        console->writeln("Setting up the application");

        JSON::Value thing = 21;
        thing = 42;
        auto val_object = JSON::Value::object(0);
        auto& obj = val_object.to_object();

        obj["hello"] = 42;
        obj["world"] = "goo goo gaa gaa";
        obj["space"] = JSON::Value::array();
        auto& arr = obj["space"].to_array();
        for (int i = 0; i < 5; ++i)
        {
            arr.push("balls");
        }

        arr.push(JSON::Value::object());
        auto& arr_obj = arr.element_at(5).to_object();
        arr_obj["is this okay?"] = "oh yeah baby";

        console->writeln(JSON::to_string(val_object, 2));
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

            // delete mini_console;
            delete console;

            delete main_window;

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
