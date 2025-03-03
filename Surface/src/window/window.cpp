#include "window.h"

namespace Surface
{

Window* create_platform_window(const char* name, const WindowOptions& options);

void destroy_platform_window(Window& window);

Window* get_platform_window(const WindowHandle& window_handle);

bool hide_platform_window(Window& window);

bool show_platform_window(Window& window);

void update_platform_window(Window& window);

Window* Window::get_window(WindowHandle& handle)
{
    return get_platform_window(handle);
}

Window* Window::create(const char* name, const WindowOptions& options)
{
    return create_platform_window(name, options);
}

Window::~Window()
{
    destroy_platform_window(*this);
}

bool Window::hide()
{
    return hide_platform_window(*this);
}

bool Window::show()
{
    return show_platform_window(*this);
}

void Window::update()
{
    update_platform_window(*this);
}
} // namespace Surface

////////////////////////////////////////////////
//              Windows Platform              //
////////////////////////////////////////////////
#ifdef PLATFORM_WINDOWS

// TODO: include the exact windows headers actually used by Surface Window
//       I've gone ahead and manually included the transitive headers anyway
//       But Windows.h surely includes far more stuff than we use here
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <basetsd.h>
#include <libloaderapi.h>
#include <minwindef.h>

namespace Surface
{

struct WindowHandle
{
    HWND* handle;
};

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);

Window* create_platform_window(const char* name, const WindowOptions& options)
{
    HINSTANCE h_instance = GetModuleHandleA(0);

    WNDCLASSA wc = {};
    wc.hInstance = h_instance;
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = name;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hIcon = LoadIcon(h_instance, IDI_APPLICATION);
    wc.style = 0;

    // class styles
    if (options.btn_close_none)
    {
        wc.style |= CS_NOCLOSE;
    }

    RegisterClassA(&wc);

    DWORD style = 0;
    DWORD exStyle = 0;
    int x = CW_USEDEFAULT, y = CW_USEDEFAULT;
    int width = CW_USEDEFAULT, height = CW_USEDEFAULT;
    HWND parent = 0;

    if (options.parent)
    {
        parent = *(options.parent->get_handle()->handle);
    }

    if (options.positioned)
    {
        if (options.x)
        {
            x = options.x;
        }

        if (options.y)
        {
            y = options.y;
        }
    }

    if (options.width)
    {
        width = options.width;
    }

    if (options.height)
    {
        height = options.height;
    }

    // Main styles
    if (options.frame_none)
    {
        // empty
    }
    else if (options.frame_thin)
    {
        style |= WS_BORDER;
    }
    else if (options.frame_thick)
    {
        style |= WS_THICKFRAME;
    }

    if (!options.hidden)
    {
        style |= WS_VISIBLE;
    }

    if (options.minimized)
    {
        style |= WS_MINIMIZE;
    }
    else if (options.maximized)
    {
        style |= WS_MAXIMIZE;
    }

    // Don't turn on the title bar if frame_none is true
    if (!options.title_none && !options.frame_none)
    {
        style |= WS_CAPTION | WS_SYSMENU; // Must show both
    }

    // Extended styles
    if (options.pinned)
    {
        exStyle |= WS_EX_TOPMOST;
    }

    // Create our window object first so we have the pointer
    WindowHandle* window_handle = new WindowHandle{.handle = nullptr};
    Window* window = new Window(name, options.title, window_handle);

    // clang-format off
    HWND handle = CreateWindowExA(
            exStyle, // extended styles
            name, // window class name registered above
            options.title,
            style, // main styles
            x, y,
            width, height,
            parent,
            0, // Menu for window, not implemented
            h_instance,
            window // Window obj, we must pass ourselves to retrieve this during message processing
    );
    // clang-format on

    if (handle == 0)
    {
        return nullptr;
    }

    // set the window handle!!!
    // TODO: is this correct? We need to stop handle from being deleted, so I think we just copy to
    // a new handle
    window_handle->handle = new HWND(handle);

    // Show window
    // TODO: this probably doesn't respect minimized/ maximized option
    ShowWindow(handle, SW_SHOW);

    return window;
}

void destroy_platform_window(Window& window)
{
    DestroyWindow(*window.get_handle()->handle);
    window.closed = true;
}

Window* get_platform_window(const WindowHandle& window_handle)
{
    LONG_PTR ptr = GetWindowLongPtrA(*window_handle.handle, GWLP_USERDATA);
    Window* window = reinterpret_cast<Window*>(ptr);
    return window;
}

bool hide_platform_window(Window& window)
{
    return ShowWindow(*window.get_handle()->handle, SW_HIDE) != 0;
}

bool show_platform_window(Window& window)
{
    return ShowWindow(*window.get_handle()->handle, SW_SHOW) == 0;
}

void update_platform_window(Window& window)
{
    MSG msg;

    while (PeekMessageA(&msg, *window.get_handle()->handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    Window* window;
    if (msg == WM_CREATE)
    {
        CREATESTRUCTA* create = reinterpret_cast<CREATESTRUCTA*>(l_param);
        window = reinterpret_cast<Window*>(create->lpCreateParams);
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR) window);
    }
    else
    {
        window = get_platform_window({.handle = &hwnd});
    }

    switch (msg)
    {
    case WM_CLOSE:
        destroy_platform_window(*window);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_QUIT:
        window->quitting = true;
        return 0;
    default:
        return DefWindowProcA(hwnd, msg, w_param, l_param);
    }
}

} // namespace Surface
#endif
