#include "console.h"

// Implement per platform
namespace Surface
{

bool create_platform_console(Console& console);

void destroy_platform_console(Console& console);

bool flush_platform_console(Console& console);

bool is_buffered_platform_console(Console& console);

bool is_open_platform_console(Console& console);

bool write_platform_console(Console& console, char* text);

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
#include <cstring>
#include <errhandlingapi.h>
#include <fileapi.h>
#include <handleapi.h>
#include <minwinbase.h>
#include <namedpipeapi.h>
#include <processthreadsapi.h>
#include <synchapi.h>
#include <winerror.h>

namespace Surface
{

const size_t BUFFER_SIZE = 1024;
const size_t PIPE_BUFFER_SIZE = 128;

struct ConsoleHandle
{
    ~ConsoleHandle();

    PROCESS_INFORMATION* pi = nullptr;
    HANDLE* out = nullptr;
    OVERLAPPED* overlap = nullptr;

    // connection state
    bool connected = false;
    bool closed = false;
    bool connecting = false;
    bool write_pending = false;

    // buffered output while waiting for connections
    char buff[BUFFER_SIZE] = {0};
    size_t size = 0;

    /**
     * @brief Helper to buffer text
     * @param text text to buffer
     * @return true if the text fit into the buffer, false if not
     */
    bool buffer(const char* text)
    {
        // Full
        if (size == BUFFER_SIZE - 1)
        {
            return false;
        }

        size_t str_size = strnlen(text, BUFFER_SIZE + 1); // Overflowing texts get ignored

        // Don't buffer empty strings
        if (str_size == 0)
        {
            return true;
        }

        if (size + str_size > BUFFER_SIZE)
        {
            return false; // not enough space to buffer
        }

        // just memcpy, we don't want null characters
        memcpy(buff + size, text, str_size);
        size += str_size;
        return true;
    }

    /**
     * @brief Helper to pull buffered text out, add null characters if size < count
     *
     * @param count characters to pull
     * @return
     */
    void unshift(char* out, size_t count)
    {
        // Min between count and size
        size_t real_count = size < count ? size : count;

        // write out bytes
        memcpy(out, buff, real_count);
        if (count > real_count)
        {
            // write zero up to count
            memset(out + real_count, 0, count - real_count);
        }

        // reduce size
        size -= real_count;

        // copy the end of our buffer to the front
        for (size_t i = 0, k = real_count; i < size; ++i, ++k)
        {
            buff[i] = buff[k];
        }

        // memset 0 the remainder of the buffer
        memset(buff + size, 0, BUFFER_SIZE - size);
    }

    /**
     * @brief Helper to put text at the front of the buffer
     * @param text text to shift in
     * @param count how much from text to shift in
     */
    void shift(const char* text, size_t count)
    {
        // just use count as a soft-max
        if (count > BUFFER_SIZE)
        {
            count = BUFFER_SIZE;
        }

        count = strnlen(text, count);

        if (count == 0)
        {
            return;
        }

        if (count == BUFFER_SIZE)
        {
            // simply overwrite the entire buffer
            strncpy_s(buff, BUFFER_SIZE, text, BUFFER_SIZE);
            return;
        }

        // copy buffer temporarily
        char* temp = new char[BUFFER_SIZE];
        memcpy(temp, buff, BUFFER_SIZE);

        // write text to front of buffer
        strncpy_s(buff, BUFFER_SIZE, text, count);

        // write temp to buffer
        memcpy(buff + count, temp, BUFFER_SIZE - count);

        // update buffer size
        if (size + count > BUFFER_SIZE)
        {
            size = BUFFER_SIZE;
        }
        else
        {
            size += count;
        }
    }

    bool connect()
    {
        if (connected)
        {
            // Already connected
            return true;
        }

        if (closed)
        {
            // Cannot connect once closed
            return false;
        }

        if (!connecting)
        {
            // Only attempt connecting one time
            disconnect();
            return false;
        }

        int status = ConnectNamedPipe(*out, overlap);
        if (status)
        {
            // Connection failed
#ifdef DEBUG
            status = GetLastError();
#endif
            disconnect();
            return false;
        }

        status = GetLastError();
        if (status == ERROR_IO_PENDING || status == ERROR_PIPE_LISTENING)
        {
            // still waiting
            return false;
        }

        if (status == ERROR_PIPE_CONNECTED)
        {
            // connected! silly windows apis... the error is the success condition!
            connected = true;
            connecting = false;

            return true;
        }

        // Something else happened
#ifdef DEBUG
        int debug = status;
#endif
        disconnect();
        return false;
    }

    void disconnect()
    {
        connected = false;
        connecting = false;
        if (out != nullptr)
        {
            CloseHandle(*out);
            delete out;
            out = nullptr;
        }
    }
};

ConsoleHandle::~ConsoleHandle()
{
    disconnect(); // this deletes `out`
    delete pi;
    delete overlap;

    pi = nullptr;
    overlap = nullptr;
};

bool create_platform_console(Console& console)
{
    // We could use AllocConsole() but this doesn't create the user's preferred terminal.
    // So, instead we use CreateProcess() and pass a PS script that just echos a named pipe.
    // It took me way too long to discover this solution but it's exactly what I wanted.

    // Set up console pipe out handle
    ConsoleHandle* handle = console.get_handle();
    handle->out = new HANDLE();
    handle->overlap = new OVERLAPPED();
    // Overlapped needs an event object to signal
    handle->overlap->hEvent = CreateEventA(0, 1, 1, 0);
    const char* pipename = "\\\\.\\pipe\\surface-console-pipe"; // TODO: use static pipe name to
                                                                // share with below PS script

    // clang-format off
    HANDLE pipe = CreateNamedPipeA(
            pipename, 
            PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED, 
            PIPE_TYPE_MESSAGE | PIPE_NOWAIT, // Asynchronous mode to not block application
            1,
            PIPE_BUFFER_SIZE,
            0,
            0,
            0
    );
    // Copy to out pipe
    DuplicateHandle(
            GetCurrentProcess(),
            pipe,
            GetCurrentProcess(),
            handle->out,
            0,
            0,
            DUPLICATE_SAME_ACCESS
    );
    // clang-format on

    STARTUPINFOA* si = new STARTUPINFOA();
    PROCESS_INFORMATION* pi = new PROCESS_INFORMATION();

    // We must zero out the structs
    memset(si, 0, sizeof(*si));
    memset(pi, 0, sizeof(*pi));

    si->cb = sizeof(si); // It is required that this contain the struct size
    si->wShowWindow = 1;
    si->dwFlags |= STARTF_USESHOWWINDOW;

#pragma warning(                                                                                   \
    disable : 6277) // Security should not be a concern for us, developers passing user-provided
                    // strings to open a console window should be an obvious red flag.
    // clang-format off
    // Simple PS script to open a pipe and echo lines out
    const char* proc_script = "powershell.exe "
            "write-host ': Starting Console' -foregroundcolor darkgreen;"
            "$Host.UI.RawUI.WindowTitle = 'Unnamed Surface Console Window';"
            "$pipe = new-object System.IO.Pipes.NamedPipeClientStream('.', 'surface-console-pipe', [System.IO.Pipes.PipeDirection]::In);"
            "$reader = new-object System.IO.StreamReader($pipe);"
            "$closed = $false;"
            "$connected = $false;"
            "$errored = $false;"
            "try"
            "{"
                "write-host ': Connecting to application...' -foregroundcolor darkgreen;"
                "$pipe.Connect(5000);"
                "$connected = $true;"
                "write-host ': Connected!\n' -foregroundcolor darkgreen;"
                "while (1)"
                "{"
                    "$msg = $reader.ReadLine();"

                    // Test connection, ReadLine() returns immediately without a connection
                    "if (-not $pipe.IsConnected)"
                    "{"
                        "break;"
                    "}"

                    // End communications
                    "if ($msg -eq 'END')"
                    "{"
                        "$closed = $true;"
                        "break;"
                    "}"

                    // Set window title
                    "if ($msg -match '^set-title:(?<title>.+)')"
                    "{"
                        "if ($Matches.title)"
                        "{"
                            "$Host.UI.RawUI.WindowTitle = $Matches.title;"
                        "}"
                        "continue;"
                    "}"

                    // Clear window
                    "if ($msg -eq 'CLS')"
                    "{"
                        "clear-host;"
                        "continue;"
                    "}"
                    
                    // Output the message as-is
                    "write-host $msg;"
                "}"
            "}"
            "catch"
            "{"
                "$errored = $true;"
                "write-host '\n: An error occurred in the console\n' -foregroundcolor red;"
                "write-host $_ -foregroundcolor red;"
            "}"
            "finally"
            "{"
                "if ($closed)"
                "{"
                    "write-host '\n: Application ended the session' -foregroundcolor darkgreen;"
                "}"
                "elseif ($connected)"
                "{"
                    "write-host '\n: Connection lost' -foregroundcolor red;"
                "}"
                "else"
                "{"
                    "write-host '\n: Failed to connect to application' -foregroundcolor red;"
                "}"
                "write-host '\nPress any key to close this window...' -foregroundcolor darkblue;"
                "$null = [System.Console]::ReadKey();"
            "}"
        "";
    int result = CreateProcessA(
            0,
            (char*) proc_script,
            0,
            0,
            0,
            CREATE_NEW_CONSOLE,
            0,
            0,
            si,
            pi
    );
    // clang-format on
#pragma warning(default : 6277)

    // Startup Info is copied to the process, per documentation, so we should delete it immediately
    delete si;

    if (result == 0)
    {
#ifdef DEBUG
        DWORD err = GetLastError();
#endif
        return false;
    }

    // We must retain the process information in case the process needs to be closed when
    // Console is destroyed
    handle->pi = pi;

    // Initiate connection
    handle->connecting = true;
    handle->connect();

    // Send title as a special message
    console.write("set-title:");
    console.write(console.title);
    console.write("\n");

    return true;
}

void destroy_platform_console(Console& console)
{
    ConsoleHandle* handle = console.get_handle();
    if (handle == nullptr)
    {
        return;
    }

    PROCESS_INFORMATION* pi = console.get_handle()->pi;
    if (pi == nullptr)
    {
        return;
    }

    // Close the process by terminating the main thread
    if (console.attached)
    {
        // Because this is simply a spawned console, this is acceptable to do.
        // TODO: send a CTRL+C to the console process or similar to close it naturally.
        TerminateProcess(pi->hProcess, 0);
    }

    // Close our handles when we are destroyed
    handle->disconnect();
    CloseHandle(pi->hProcess);
    CloseHandle(pi->hThread);
}

bool flush_platform_console(Console& console)
{
    ConsoleHandle* handle = console.get_handle();
    if (handle == nullptr)
    {
        return true;
    }

    console.write("");
    return handle->size == 0;
}

bool is_buffered_platform_console(Console& console)
{
    ConsoleHandle* handle = console.get_handle();
    if (handle == nullptr)
    {
        return false;
    }

    return handle->size > 0;
}

bool is_open_platform_console(Console& console)
{
    ConsoleHandle* handle = console.get_handle();
    if (handle == nullptr)
    {
        return false;
    }

    return handle->pi != nullptr;
}

bool write_platform_console(Console& console, const char* text)
{
    ConsoleHandle* handle = console.get_handle();
    if (handle == nullptr)
    {
        return false;
    }

    if (handle->closed)
    {
        return false; // Cannot write to closed pipe
    }

    if (!handle->connected)
    {
        if (!handle->connecting)
        {
            // Assume there was a problem, not connected and not waiting is bad
            handle->closed = true;
            handle->disconnect();
            return false;
        }

        // Check status
        if (!handle->connect())
        {
            // We can buffer if still pending
            int status = GetLastError();
            if (status == ERROR_IO_PENDING || status == ERROR_PIPE_LISTENING)
            {
                handle->buffer(text);
            }
            return false;
        }
    }

    if (handle->write_pending)
    {
        if (HasOverlappedIoCompleted(handle->overlap))
        {
            handle->write_pending = false;
        }
        else
        {
            // Buffer if we aren't ready to send
            handle->buffer(text);
            return false;
        }
    }

    // Ready to send chunks
    bool text_processed = false;
    bool from_text = false;
    char* buff = nullptr;

    while (1)
    {
        int status;
        size_t count = 0;
        DWORD written = 0;
        if (handle->size > 0)
        {
            // Write buffered data first
            if (buff == nullptr)
            {
                buff = new char[PIPE_BUFFER_SIZE]{0};
            }

            handle->unshift(buff, PIPE_BUFFER_SIZE);
            count = strnlen(buff, PIPE_BUFFER_SIZE);

            // Send `count` bytes contained in `buff`
#pragma warning(disable : 4267) // Count will never be larger than PIPE_BUFFER_SIZE
            status = WriteFile(*handle->out, buff, count, &written, handle->overlap);
#pragma warning(default : 4267)
            from_text = false;
        }
        else
        {
            // Only process text parameter once
            if (text_processed)
            {
                break;
            }
            text_processed = true;

            count = strnlen(text, (size_t) -1);

            // If we have too much, buffer it instead and let the above handle it
            if (count > PIPE_BUFFER_SIZE)
            {
                handle->buffer(text);
                continue;
            }

            // Send from text directly
#pragma warning(disable : 4267) // Count will never be larger than PIPE_BUFFER_SIZE
            status = WriteFile(*handle->out, text, count, &written, handle->overlap);
#pragma warning(default : 4267)
            from_text = true;
        }

        // This can complete immediately, so check for that
        if (status)
        {
            if (written == count)
            {
                // Keep sending
                continue;
            }

            // Success, but not enough was written... back up the buffer to try again later as we
            // were just sending too quick
            if (from_text)
            {
                handle->shift(text + written, count - written);
            }
            else
            {
                handle->shift(buff + written, count - written);
            }

            break;
        }

        if (status == 0)
        {
            status = GetLastError();
            if (status == ERROR_IO_PENDING)
            {
                // Need to wait
                handle->write_pending = true;
                break;
            }
            else if (status == ERROR_NO_DATA)
            {
                // Pipe is being closed, disconnect
                handle->closed = true;
                handle->disconnect();
                delete[] buff;
                return false;
            }
        }

#ifdef DEBUG
        status = GetLastError();
#endif

        // Something bad happened, disconnect
        handle->closed = true;
        handle->disconnect();

        delete[] buff;
        return false;
    }

    delete[] buff;

    if (!text_processed)
    {
        // Text has not been processed, must buffer it
        handle->buffer(text);
        return false;
    }

    // An empty buffer means all text was sent on this call
    return handle->size == 0;
}
} // namespace Surface

#endif

namespace Surface
{

Console* Console::create(const char* title, bool attach)
{
    Console* console = new Console(title, attach, new ConsoleHandle());

    if (create_platform_console(*console))
    {
        return console;
    }

    delete console;
    return nullptr;
}

Console::~Console()
{
    destroy_platform_console(*this);
    delete handle;
}

void Console::close()
{
    destroy_platform_console(*this);
    delete handle;
}

bool Console::write(const char* text)
{
    return write_platform_console(*this, text);
}

bool Console::flush()
{
    return flush_platform_console(*this);
}

bool Console::is_buffered()
{
    return is_buffered_platform_console(*this);
}

bool Console::is_open()
{
    return is_open_platform_console(*this);
}

} // namespace Surface
