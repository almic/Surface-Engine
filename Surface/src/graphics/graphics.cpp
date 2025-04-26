#include "graphics.h"

#include "directx12.h"

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

void BlankRenderEngine::clear_commands()
{
}

} // namespace Surface::Graphics
