#pragma once

#include "error.h"

// Base graphics API
namespace Surface::Graphics
{

// Supported graphics APIs
enum API : unsigned char
{
    DirectX12
};

// The render engine that is used to render things
struct RenderEngine;

// A shader object that transforms data during a render pass
struct Shader;

// Contains data used for rendering geometry
struct GeometryData;

struct RenderEngine
{
    // Create a render engine for the given graphics api
    static RenderEngine* create(API api);

    // Provide the window handle this render engine will render to, can be called anytime to reuse a
    // single engine for multiple windows. Returns `false` if the window was not bound, failure will
    // also effectively disable the `render()` method.
    virtual bool bind_window(void* native_window_handle) = 0;

    // Dispatches all render commands to the GPU for the current window
    virtual bool render() = 0;

    // Clears the render command list
    virtual void clear_commands() = 0;

    // Get the last error raised
    virtual const Error& get_last_error() const
    {
        return last_error;
    }

    virtual void set_clear_color(const float (&color)[4]) = 0;

    virtual bool resize(unsigned int width, unsigned int height) = 0;

    virtual const char* get_device_name() const = 0;

  public: // Operators
    inline operator bool() const
    {
        return !get_last_error();
    }

  public: // Methods not intended for direct use
    // Initializes anything needed prior to any other engine calls
    virtual void initialize() {};

  public: // Constructor/ destructor methods
    RenderEngine() {};
    virtual ~RenderEngine() {};

    // To spare resource usage, strictly disallow copying, which may be non-trivial on some
    // platforms and graphics apis
    RenderEngine(const RenderEngine& other) = delete;
    RenderEngine& operator=(const RenderEngine& other) = delete;

    // Moving should be trivial for all implementations
    RenderEngine(RenderEngine&& other) = default;
    RenderEngine& operator=(RenderEngine&& other) noexcept = default;

  protected:
    Error last_error = Error::none();
};

struct BlankRenderEngine : public RenderEngine
{
    BlankRenderEngine()
    {
        last_error = Error::create("This is a blank, no-op Render Engine.", Error_Generic);
    }

    bool bind_window(void* native_window_handle) override;
    bool render() override;
    void clear_commands() override;
    void set_clear_color(const float (&color)[4]) override;
    bool resize(unsigned int width, unsigned int height) override;
    const char* get_device_name() const override;
};

} // namespace Surface::Graphics
