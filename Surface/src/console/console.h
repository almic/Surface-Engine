#pragma once

namespace Surface
{
/**
 * @brief Platform specific handle struct
 */
struct ConsoleHandle;

/**
 * @brief A very simple console, it should be used by applications that start without displaying a
 * console window. The spawned console process will share stdio buffers (stdout/ stdin/ stderr) and
 * should be the system user's preferred terminal application.
 */
class Console
{
  private:
    ConsoleHandle* handle;

    Console(const char* title, bool attached, ConsoleHandle* handle)
        : title(title), attached(attached), handle(handle) {};

  public:
    /**
     * @brief Create a console window sharing std buffers, optionally attaching to it.
     *
     * @param title title for the console window, defaults to the platform terminal executable name
     * @param attach when this Console is destroyed, the spawned console will be closed.
     */
    static Console* create(const char* title, bool attach = false);

    ~Console();

    // If this console is attached, when Console is destroyed, the spawned console will be closed.
    bool attached;

    // Title of the console
    const char* title;

    /**
     * @brief Clears the console screen
     */
    void clear()
    {
        writeln("CLS");
    }

    /**
     * @brief Closes the console window immediately
     */
    void close();

    /**
     * @brief Ends the console connection, but leaves the window open so it can still be read
     */
    void end()
    {
        writeln("END");
    }

    // Get the console handle
    ConsoleHandle* get_handle()
    {
        return handle;
    }

    /**
     * @brief Check if console output is currently buffered.
     *
     * Depending on the platform, this may always return false.
     *
     * Use flush() to attempt to clear the buffer when this returns true.
     *
     * @return true if text is buffered, false otherwise.
     */
    bool is_buffered();

    /**
     * @brief Check if the console is open
     * @return true if open, false otherwise.
     */
    bool is_open();

    /**
     * @brief Writes output to console, buffering in some cases. Regardless of platform, you are
     * guaranteed 1024 bytes of buffer. Attempting to write more than the buffer can hold will
     * simply be ignored.
     *
     * Depending on the platform, a connection may not be open yet and text will be buffered. The
     * buffer size depends on the platform. Console writing will attempt to flush the entire buffer
     * on each call. A return value of true means the buffer was flushed to the console.
     *
     * You can use flush() when text may be buffered.
     *
     * @param text text to write
     * @return true if text was written, false if it was buffered or otherwise failed.
     */
    bool write(const char* text);

    /**
     * @brief Writes a line of output to console. Please read the documentation for `write()` for
     * more information.
     *
     * @param text text to write
     * @return true if text was written, false if it was buffered or otherwise failed.
     */
    inline bool writeln(const char* text)
    {
        // Be simple
        bool result = write(text);
        result &= write("\n");
        return result;
    }

    /**
     * @brief Flushes any buffered output to console.
     *
     * Depending on the platform, this may not do anything at all. Some platforms may only display
     * when a linebreak (\n) is sent.
     *
     * @return true if the buffer was emptied by this call
     */
    bool flush();

    /**
     * @brief Set the title of the console window
     * @param title title to set
     */
    inline void set_title(const char* title)
    {
        this->title = title;
        write("set-title:");
        write(this->title);
        write("\n");
    }

    /**
     * @brief If there is buffered text, calls the flush() method
     */
    inline void update()
    {
        if (is_buffered())
        {
            flush();
        }
    }
};
} // namespace Surface
