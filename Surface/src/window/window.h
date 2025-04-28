#pragma once

// TODO: transparency stuff like this:
//       https://stackoverflow.com/questions/3970066/creating-a-transparent-window-in-c-win32
//       https://faithlife.codes/blog/2008/09/displaying_a_splash_screen_with_c_part_ii/

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

#ifdef PLATFORM_WINDOWS

struct WindowHandle
{
    // window handle
    void* handle = nullptr;

    // if this handle is to a console window
    bool is_console = false;

    // For non-blocking resizes/ moves
    unsigned char resize_skip = 0; // performance use
    unsigned int resizing_moving;
    long long mouse_pos;

    // Used to reset window style for fullscreen trasitions
    unsigned int style = 0;
};

#else

struct WindowHandle
{
};

#endif

class Window
{
  private:
    // The console window of the application
    inline static Window* console_window;

  public: // Typedefs
    /**
     * @brief Title bar hit test method, coordinates are in client space, where (0, 0) is the
     * top-left corner of the window.
     *
     * @return true if the title bar is hit and the native window should handle it, such as to
     * resize or move the window
     */
    typedef bool (*TitlebarHitTest)(Window* window, unsigned int x_pos, unsigned int y_pos);

    /**
     * @brief Callback when a window is resized.
     */
    typedef void (*ResizeCallback)(Window* window);

    /**
     * @brief Window dimensions
     */
    struct Rect
    {
        int x;
        int y;
        unsigned int width;
        unsigned int height;
    };

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

    template <unsigned int height>
    static bool static_title_bar_test(Window*, unsigned int, unsigned int y_pos)
    {
        return !(height < y_pos);
    }

    ~Window();

  private:
    // Internal
    Window(const char* name, const char* title, WindowHandle handle)
        : name(name), title(title), handle(handle) {};

    // Window width
    unsigned int width = 0;
    // Window height
    unsigned int height = 0;

    // Window x position
    int x = 0;
    // Window y position
    int y = 0;

    // Last window rect, used for fullscreen transitions
    Rect last_rect;
    bool m_is_fullscreen = false;

    // Window internal name
    const char* name;

    // Window title displayed to user
    const char* title;

    // Window handle pointer, for platform implementations
    WindowHandle handle;

    // Hit test method for the title bar
    TitlebarHitTest m_title_bar_hit_test = nullptr;

    // Resize callback
    ResizeCallback m_resize_callback = nullptr;

  public:
    // If the application has been quit via this window and should terminate
    bool quitting = false;

    // If the window has been closed
    bool closed = false;

    // Get the window handle
    inline const WindowHandle& get_handle() const
    {
        return handle;
    }

    inline WindowHandle& get_handle()
    {
        return handle;
    }

    // Get the native platform window handle, used by few APIs
    inline void* get_native_handle() const
    {
#ifdef PLATFORM_WINDOWS
        return handle.handle;
#else
        return nullptr;
#endif
    }

    // If this window uses custom frames
    bool no_frame = false;

    inline void set_title_bar_hit_test(TitlebarHitTest method)
    {
        m_title_bar_hit_test = method;
    }

    inline bool title_bar_hit_test(unsigned int x_pos, unsigned int y_pos)
    {
        if (m_title_bar_hit_test)
        {
            return m_title_bar_hit_test(this, x_pos, y_pos);
        }
        return false;
    }

    inline void set_resize_callback(ResizeCallback method)
    {
        m_resize_callback = method;
    }

    inline void on_resize()
    {
        if (m_resize_callback)
        {
            m_resize_callback(this);
        }
    }

    inline bool is_fullscreen() const
    {
        return m_is_fullscreen;
    }

    // Set window fullscreen mode
    bool fullscreen(bool enable);

    // Hide this window, returning true if the window was previously shown
    bool hide();

    // Show this window, returning true if the window was previously hidden
    bool show();

    // Update this window
    void update();

    // Get the window position and size
    inline Rect rect() const
    {
        return {x, y, width, height};
    }

  private:
    // Allow private access from these functions
    friend bool create_platform_window(Window& window, const char* name,
                                       const WindowOptions& options);
    friend void destroy_platform_window(Window& window);
    friend bool get_platform_console_window(Window& window);
    friend Window* get_platform_window(const WindowHandle& window_handle);
    friend bool hide_platform_window(Window& window);
    friend bool show_platform_window(Window& window);
    friend void update_platform_window(Window& window);
    friend void set_platform_window_rect(Window& window, const Rect& rect);
    friend bool set_platform_window_fullscreen(Window& window, bool enable);
};

} // namespace Surface
