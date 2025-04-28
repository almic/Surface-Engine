#include "app/app.h"
#include "console/console.h"
#include "file/file.h"
#include "graphics/dx12/directx12.h"
#include "graphics/graphics.h"
#include "time/time.h"
#include "window/window.h"

#include "json/json.h"
#include <functional>
#include <iostream>
#include <thread>

#undef NULL

using Surface::JSON::json;

// Static because we only use one and it makes static callbacks possible
static Surface::Graphics::RenderEngine* render_engine = nullptr;

// Static target console for logging
Surface::Console* console = nullptr;

static void LOG(const char* message)
{
    if (!console)
    {
        return;
    }

    console->writeln(message);
}

class SandboxApp : public Surface::App
{
    Surface::Window* main_window = nullptr;
    Surface::Window* mini_console = nullptr;
    Surface::Console* console = nullptr;
    std::function<void(const char*)> log;

    float clear_color[4] = {1.0f, 0.0f, 0.0f, 1.0f};

    static bool rotate_color(float (&color)[4], double delta)
    {
        static double accumulator = 0;
        static float step = 0.002f;

        accumulator += delta;

        if (accumulator < 0.01)
        {
            return false;
        }

        accumulator -= 0.01;

        // Switch points
        for (char i = 0; i < 3; ++i)
        {
            if (color[i] == 1.0f)
            {
                color[i] -= step;
                color[(i + 1) % 3] = step;
                return true;
            }
        }

        // Flow points
        for (char i = 0; i < 3; ++i)
        {
            char k = (i + 1) % 3;
            if (color[i] > 0.0f && color[k] > 0.0f)
            {
                color[k] += step;
                color[i] -= step;

                if (color[k] > 1.0f || color[i] < 0.0f)
                {
                    color[k] = 1.0f;
                    color[i] = 0.0f;
                }

                break;
            }
        }

        return true;
    }

    static void resize(Surface::Window* window)
    {
        if (!render_engine || !render_engine->bind_window(window->get_native_handle()))
        {
            return;
        }

        auto rect = window->rect();
        if (!render_engine->resize(rect.width, rect.height))
        {
            LOG(render_engine->get_last_error().get_message());
        }
        else
        {
            LOG("Resized to:");
            LOG(json::to_string(rect.width).string());
            LOG(json::to_string(rect.height).string());
        }
    }

    void create_windows()
    {
        Surface::WindowOptions options{
            // clang-format off
            .title = "Hello World",
            .frame_none = true,
            .title_none = true,
            // clang-format on
        };

        main_window = Surface::Window::create("main", options);

        main_window->set_title_bar_hit_test(Surface::Window::static_title_bar_test<30>);

        // mini_console = Surface::Window::get_console_window();
        console = Surface::Console::create("Sandbox Console", false);
    }

    void setup() override
    {
        Surface::Time::Timer timer;
        create_windows();

        log = std::bind(&Surface::Console::writeln, console, std::placeholders::_1);
        ::console = console;

        log("Setting up the application");

        auto user_data = Surface::FS::user_app_data_path();
        auto app_data = Surface::FS::sys_app_data_path();

        log("User data path:");
        log(user_data.string().c_str());
        log("App data path:");
        log(app_data.string().c_str());

        render_engine = Surface::Graphics::RenderEngine::create(Surface::Graphics::DirectX12);
        if (!*render_engine)
        {
            log("Render Engine encountered an error!");
            log(render_engine->get_last_error().get_message());
        }
        else
        {
            if (render_engine->bind_window(main_window->get_native_handle()))
            {
                log("Render Engine bound to main window!");
                auto device_name = render_engine->get_device_name();

                if (!device_name)
                {
                    log("Unknown device");
                }
                else
                {
                    log(device_name);
                }

                main_window->set_resize_callback(resize);
            }
            else
            {
                log("Render Engine failed to bind to window!");
                log(render_engine->get_last_error().get_message());
            }

            render_engine->set_clear_color(clear_color);
        }

        set_max_delta_time(0.05);

        timer.log_to(log);
    }

    void update() override
    {
        console->update();
        main_window->update();

        if (main_window->closed || main_window->quitting)
        {
            log("Main window closed, stopping.");

            stop(true);
            return;
        }

        double delta = get_delta_time();

        if (rotate_color(clear_color, delta))
        {
            render_engine->set_clear_color(clear_color);
        }
    };

    void render() override
    {
        if (!render_engine || !*render_engine)
        {
            log("Render Engine out-of-order, stopping.");

            stop(true);
            return;
        }

        static uint64_t frames = 0;
        static double fps_accum = 0.0f;

        double delta = get_delta_time();

        fps_accum += delta;
        if (fps_accum > 1.0)
        {
            // Use json string conversion
            json fps = frames / fps_accum;
            log(json::to_string(fps).string());

            fps_accum = 0.0f;
            frames = 0;
        }

        if (!render_engine->render())
        {
            log(render_engine->get_last_error().get_message());
            stop(true);
        }

        ++frames;
    }

    void teardown() override
    {
        if (render_engine)
        {
            delete render_engine;

#ifdef PLATFORM_WINDOWS
#ifdef DEBUG
            Surface::Graphics::DX12RenderEngine::debug_report_objects();
#endif
#endif
        }

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
