#include "graphics.h"

// Engine implementations
#include "dx12/directx12.h"

namespace Surface::Graphics
{

RenderEngine* RenderEngine::create(API api)
{
    switch (api)
    {
    case Surface::Graphics::DirectX12:
    {
#ifdef PLATFORM_WINDOWS
        return new DX12RenderEngine();
#else
        break;
#endif
    }
    default:
    {
        break;
    }
    }

    return new BlankRenderEngine();
}

bool BlankRenderEngine::bind_window(void* native_window_handle)
{
    return false;
}

bool BlankRenderEngine::render()
{
    return false;
}

void BlankRenderEngine::clear_commands() {};
void BlankRenderEngine::set_clear_color(const float (&color)[4]) {};

bool BlankRenderEngine::resize(unsigned int width, unsigned int height)
{
    return false;
}

const char* BlankRenderEngine::get_device_name() const
{
    return "No Device";
}

} // namespace Surface::Graphics
