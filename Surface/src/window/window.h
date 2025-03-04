#pragma once

namespace Surface
{

// C++ is weird?
class Window;

/**
 * @brief Options used when constructing windows. Some platforms support changing these after the
 * window is created, some do not.
 */
struct WindowOptions
{
    // Window title displayed to user
    const char* title;

    // Initial window width. Set to zero to use a platform default value.
    int width = 0;

    // Initial window height. Set to zero to use a platform default value.
    int height = 0;

    // Initial window x position. Must set positioned = true.
    int x = 0;

    // Initial window y position. Must set positioned = true.
    int y = 0;

    // The parent window of this new window
    Window* parent;

    // When true, hides the window close button
    bool btn_close_none = false;

    // When true, use a thin frame style for the window
    bool frame_thin = false;

    // When true, use a thick frame style for the window
    bool frame_thick = false;

    // When true, use a borderles style for the window
    bool frame_none = false;

    // When true, the window is initially hidden. This is not the same as minimized.
    bool hidden = false;

    // When true, the window is initially maximized. This is not the same as fullscreen.
    bool maximized = false;

    // When true, the window is initially minimized. This is not the same as hidden.
    bool minimized = false;

    // When true, the window is initially pinned. This will display the window above non-pinned
    // windows.
    bool pinned = false;

    // When true, uses the given x and y for the initial window position. Otherwise, platform
    // defaults are used.
    bool positioned = false;

    // When true, the window will not have a title bar.
    bool title_none = false;
};

/**
 * @brief Platform specific handle struct
 */
struct WindowHandle;

class Window
{
  private:
    // The console window of the application
    inline static Window* console_window;

  public:
    // Retrieve a created window from its handle
    static Window* get_window(WindowHandle& handle);

    /**
     * @brief Retrieve a console window for the application. If a console window does not exist, one
     * will be created and the applications stdio will be directed to it.
     * @return console window
     */
    static Window* get_console_window();

    // Create a window
    static Window* create(const char* name, const WindowOptions& options);

    ~Window();

  private:
    // Internal
    Window(const char* name, const char* title, WindowHandle* handle)
        : name(name), title(title), handle(handle) {};

    // Window width
    unsigned int width = 0;
    // Window height
    unsigned int height = 0;

    // Window x position
    int x = 0;
    // Window y position
    int y = 0;

    // Window internal name
    const char* name;

    // Window title displayed to user
    const char* title;

    // Window handle pointer, for platform implementations
    WindowHandle* handle;

  public:
    // If the application has been quit via this window and should terminate
    bool quitting = false;

    // If the window has been closed
    bool closed = false;

    // Get the window handle
    WindowHandle* get_handle()
    {
        return handle;
    }

    // Hide this window, returning true if the window was previously shown
    bool hide();

    // Show this window, returning true if the window was previously hidden
    bool show();

    // Update this window
    void update();
};
} // namespace Surface
