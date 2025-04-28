#include "window.h"

// Implement per platform
namespace Surface
{

bool create_platform_window(Window& window, const char* name, const WindowOptions& options);

void destroy_platform_window(Window& window);

bool get_platform_console_window(Window& window);

Window* get_platform_window(const WindowHandle& window_handle);

bool hide_platform_window(Window& window);

bool show_platform_window(Window& window);

void update_platform_window(Window& window);

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

#include <windows.h>
// ^ Must be first

#include <basetsd.h>
#include <consoleapi.h>
#include <consoleapi3.h>
#include <cstdio>
#include <dwmapi.h>
#include <libloaderapi.h>
#include <windef.h>

static inline POINT to_point(LPARAM l_param)
{
    return {(int) (short) ((WORD) (((DWORD_PTR) (l_param)) & 0xffff)),
            (int) (short) ((WORD) ((((DWORD_PTR) (l_param)) >> 16) & 0xffff))};
}

static inline LPARAM from_point(POINT point)
{
    return (static_cast<LPARAM>(point.y) << 16) | (static_cast<LPARAM>(point.x) & 0xFFFF);
}

static inline bool is_left_mouse()
{
    short stat;
    if (GetSystemMetrics(SM_SWAPBUTTON))
    {
        stat = GetAsyncKeyState(VK_RBUTTON);
    }
    else
    {
        stat = GetAsyncKeyState(VK_LBUTTON);
    }
    return stat & 0x8000; // high bit contains true value
}

namespace Surface
{

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);

bool create_platform_window(Window& window, const char* name, const WindowOptions& options)
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

    {
        if (options.parent)
        {
            parent = (HWND) (options.parent->get_handle().handle);
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
            if (options.title_none)
            {
                window.no_frame = true;

                // Enables WIN+ARROW key for window snapping/ maximize/ minimize
                // TODO: find out how to hack it to enable drag snapping
                style |= WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
            }
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
    }

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
            &window // Window obj, we must pass ourselves to retrieve this during message processing
    );
    // clang-format on

    if (handle == 0)
    {
        return false;
    }

    // set the window handle!!!
    // TODO: is this correct?
    window.get_handle().handle = handle;

    // Show window
    // TODO: this probably doesn't respect minimized/ maximized option
    ShowWindow(handle, SW_SHOW);

    return true;
}

void destroy_platform_window(Window& window)
{
    if (window.get_handle().is_console)
    {
        // Hiding the console is equivalent to closing it
        window.hide();
        return;
    }

    DestroyWindow((HWND) window.get_handle().handle);
    window.closed = true;
}

bool get_platform_console_window(Window& window)
{
    HWND handle = GetConsoleWindow();
    if (handle == 0)
    {
        AllocConsole();
        handle = GetConsoleWindow();

        if (handle == 0)
        {
            return false;
        }
    }

    // TODO: is this correct?
    window.get_handle().handle = handle;

    // attach stdio
    freopen_s((FILE**) stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**) stderr, "CONOUT$", "w", stderr);
    freopen_s((FILE**) stdin, "CONIN$", "r", stdin);

    return true;
}

Window* get_platform_window(const WindowHandle& window_handle)
{
    LONG_PTR ptr = GetWindowLongPtrA((HWND) window_handle.handle, GWLP_USERDATA);
    if (ptr == 0)
    {
        return nullptr;
    }
    Window* window = reinterpret_cast<Window*>(ptr);
    return window;
}

bool hide_platform_window(Window& window)
{
    if (window.get_handle().is_console)
    {
        if (window.closed)
        {
            return false;
        }

        FreeConsole();
        window.closed = true;
        return true;
    }

    return ShowWindow((HWND) window.get_handle().handle, SW_HIDE) != 0;
}

bool show_platform_window(Window& window)
{
    if (window.get_handle().is_console)
    {
        if (!window.closed)
        {
            return false;
        }

        // Calling get will ensure console is open
        get_platform_console_window(window);
        return true;
    }
    return ShowWindow((HWND) window.get_handle().handle, SW_SHOW) == 0;
}

void update_platform_window(Window& window)
{
    if (window.get_handle().is_console)
    {
        return;
    }

    // Perform window resizing/ moving first
    auto& handle = window.get_handle();
    do
    {
        if (!handle.resizing_moving)
        {
            break;
        }

        // Performance: skip 9/10 calls, as these functions are expensive
        // On my system, at the time of this commit, FPS starts at around 4200.
        // This drops ~7x (to 620FPS) when moving, and ~13x (to 320FPS) when resizing.
        // With this code, moving drops ~2x (to 1970FPS), and resizing drops ~3x (to 1380FPS).
        // Doing this gives a clear performance boost.

        ++handle.resize_skip;
        if (handle.resize_skip < 9)
        {
            break;
        }

        handle.resize_skip = 0;

        // Test if mouse is held, if the user drags quickly or above the window and releases, we
        // don't get the "BUTTONUP" message

        if (!is_left_mouse())
        {
            handle.resizing_moving = 0;
            break;
        }

        POINT pos;
        GetCursorPos(&pos);

        POINT prev = to_point(handle.mouse_pos);

        POINT offset{
            pos.x - prev.x,
            pos.y - prev.y,
        };

        if (offset.x == 0 && offset.y == 0)
        {
            break;
        }

        handle.mouse_pos = from_point(pos);

        // clang-format off
        enum region : unsigned int { left = 1, right = 2, top = 4, bottom = 8, caption = 16 };

        // clang-format on

        RECT rect;
        GetWindowRect((HWND) handle.handle, &rect);

        if (handle.resizing_moving & caption)
        {
            SetWindowPos((HWND) handle.handle, 0, rect.left + offset.x, rect.top + offset.y, 0, 0,
                         SWP_NOSIZE | SWP_NOZORDER);
            break;
        }

        if (handle.resizing_moving & top)
        {
            rect.top += offset.y;
        }
        else if (handle.resizing_moving & bottom)
        {
            rect.bottom += offset.y;
        }

        if (handle.resizing_moving & left)
        {
            rect.left += offset.x;
        }
        else if (handle.resizing_moving & right)
        {
            rect.right += offset.x;
        }

        SetWindowPos((HWND) handle.handle, 0, rect.left, rect.top, rect.right - rect.left,
                     rect.bottom - rect.top, SWP_NOZORDER);
        break;
    }
    while (false);

    MSG msg;
    while (PeekMessageA(&msg, (HWND) handle.handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    Window* window = nullptr;
    if (msg == WM_CREATE)
    {
        CREATESTRUCTA* create = reinterpret_cast<CREATESTRUCTA*>(l_param);
        window = reinterpret_cast<Window*>(create->lpCreateParams);
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR) window);
    }
    else
    {
        window = get_platform_window({.handle = hwnd});
        if (window == nullptr)
        {
            return DefWindowProcA(hwnd, msg, w_param, l_param);
        }
    }

    // clang-format off
    enum region : unsigned int { left = 1, right = 2, top = 4, bottom = 8, caption = 16 };

    // clang-format on

    switch (msg)
    {
    case WM_ACTIVATE:
    {
        if (!window->no_frame)
        {
            break;
        }

        MARGINS margins = {0};
        HRESULT result = DwmExtendFrameIntoClientArea(hwnd, &margins);
        if (FAILED(result))
        {
            // empty
        }

        break; // Continue to DefWindowProc
    }
    case WM_CREATE:
    {
        if (!window->no_frame)
        {
            break;
        }

        // Simulate a window resize to force the NCCALCSIZE message
        SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
        return 0;
    }
    case WM_CLOSE:
    {
        destroy_platform_window(*window);
        return 0;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    case WM_QUIT:
    {
        window->quitting = true;
        return 0;
    }
    case WM_NCCALCSIZE:
    {
        if (window->no_frame && w_param == 1)
        {
            return 0;
        }
        break;
    }
    case WM_NCHITTEST:
    {
        if (!window->no_frame)
        {
            break;
        }

        // Handle frame hit-test
        static RECT border{5, 5, 5, 5};
        POINT pos = to_point(l_param);

        ScreenToClient(hwnd, &pos);

        // Borders first, for resizing
        RECT rect;
        GetClientRect(hwnd, &rect);

        // Modified from Cherno's glfw fork, written by Mohit Sethi (GloriousPtr)
        int hit = 0;

        // clang-format off
        if (pos.x <= border.left)                      hit |= left;
        else if (pos.x >= rect.right - border.right)   hit |= right;

        if (pos.y <= border.top)                       hit |= top;
        else if (pos.y >= rect.bottom - border.bottom) hit |= bottom;

        if (hit & top)
        {
            if (hit & left)  return HTTOPLEFT;
            if (hit & right) return HTTOPRIGHT;
                             return HTTOP;
        }

        if (hit & bottom)
        {
            if (hit & left)  return HTBOTTOMLEFT;
            if (hit & right) return HTBOTTOMRIGHT;
                             return HTBOTTOM;
        }

        if (hit & left)      return HTLEFT;
        if (hit & right)     return HTRIGHT;
        // clang-format on

        // Try user title bar hit test
        if (window->title_bar_hit_test(pos.x, pos.y))
        {
            return HTCAPTION;
        }

        return HTCLIENT;
    }
    case WM_NCLBUTTONDOWN:
    {
        // Moving / Resizing on Windows with DefWindowProc uses an infinite loop that polls for
        // mouse updates until the button is released. This blocks user code which can be very
        // annoying and even problematic.
        //
        // DefWindowProc must do it this way to function without state, but we have state and can
        // yeild back to user code immediately.

        auto& hit = window->get_handle().resizing_moving;
        switch (GET_NCHITTEST_WPARAM(w_param))
        {
        case HTCAPTION:
        {
            hit = caption;
            break;
        }
        case HTLEFT:
        {
            hit = left;
            break;
        }
        case HTRIGHT:
        {
            hit = right;
            break;
        }
        case HTTOP:
        {
            hit = top;
            break;
        }
        case HTTOPLEFT:
        {
            hit = top | left;
            break;
        }
        case HTTOPRIGHT:
        {
            hit = top | right;
            break;
        }
        case HTBOTTOM:
        {
            hit = bottom;
            break;
        }
        case HTBOTTOMLEFT:
        {
            hit = bottom | left;
            break;
        }
        case HTBOTTOMRIGHT:
        {
            hit = bottom | right;
            break;
        }
        }

        if (hit == 0)
        {
            // Let DefWindowProc handle it
            break;
        }

        // We will begin moving the window / resizing as needed
        window->get_handle().mouse_pos = l_param;
        return 0;
    }
    case WM_NCLBUTTONUP:
    {
        window->get_handle().resizing_moving = 0;
        return 0;
    }
    }

    LRESULT result = DefWindowProcA(hwnd, msg, w_param, l_param);
    return result;
}

} // namespace Surface
#endif

namespace Surface
{

Window* Window::get_window(WindowHandle& handle)
{
    return get_platform_window(handle);
}

Window* Window::get_console_window()
{
    if (!console_window)
    {
        console_window = new Window("console", "console", {.is_console = true});

        if (!get_platform_console_window(*console_window))
        {
            delete console_window;
            console_window = nullptr;
        }
    }

    return console_window;
}

Window* Window::create(const char* name, const WindowOptions& options)
{
    Window* window = new Window(name, options.title, {});

    if (create_platform_window(*window, name, options))
    {
        return window;
    }

    delete window;
    return nullptr;
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

Window::~Window()
{
    destroy_platform_window(*this);
}

} // namespace Surface
