#include "directx12.h"

// Windows only
#ifdef PLATFORM_WINDOWS

// Because using shared_ptr wasn't "Microsoft" enough, they require CamelCase!
template <typename T> using ptr = Microsoft::WRL::ComPtr<T>;

// Feature level
static inline constexpr D3D_FEATURE_LEVEL FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_0;

// Lifted from:
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/UWP/D3D12HelloWorld/src/HelloTriangle/DXSample.cpp
static void GetHardwareAdapter(IDXGIFactory7* factory, IDXGIAdapter4** adapter_ptr,
                               bool high_performance)
{

    *adapter_ptr = nullptr;
    ptr<IDXGIAdapter4> adapter;
    for (UINT index = 0; SUCCEEDED(factory->EnumAdapterByGpuPreference(
             index,
             high_performance ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
                              : DXGI_GPU_PREFERENCE_UNSPECIFIED,
             IID_PPV_ARGS(&adapter)));
         ++index)
    {
        DXGI_ADAPTER_DESC3 desc;
        if (FAILED(adapter->GetDesc3(&desc)))
        {
            continue;
        }

        if (desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
        {
            // Don't select software adapters
            continue;
        }

        if (SUCCEEDED(
                D3D12CreateDevice(adapter.Get(), FEATURE_LEVEL, __uuidof(ID3D12Device), nullptr)))
        {
            break;
        }
    }

    if (adapter.Get() == nullptr)
    {
        // Fallback to first supported adapter
        for (UINT index = 0;
             SUCCEEDED(factory->EnumAdapters1(index, &((ptr<IDXGIAdapter1>) adapter))); ++index)
        {
            DXGI_ADAPTER_DESC1 desc;
            if (FAILED(adapter->GetDesc1(&desc)))
            {
                continue;
            }

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select software adapters
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), FEATURE_LEVEL, __uuidof(ID3D12Device),
                                            nullptr)))
            {
                break;
            }
        }
    }

    *adapter_ptr = adapter.Detach();
}

namespace Surface::Graphics
{

DX12RenderEngine::DX12RenderEngine()
{
    initialize();
}

bool DX12RenderEngine::bind_window(void* native_window_handle)
{
    if (m_window_targets.contains((HWND) native_window_handle))
    {
        // TODO: set whatever needs to be set here
        return true;
    }

    RenderTarget target;
    HRESULT result;

    // Create swap chain
    {
        DXGI_SWAP_CHAIN_DESC1 desc{};
        desc.BufferCount = BUFFER_COUNT;

        // Automatic size based on window dimensions
        // TODO: support runtime configuration of buffer dimensions
        desc.Width = 0;
        desc.Height = 0;

        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.SampleDesc.Count = 1; // Must set to disable multisampling

        ptr<IDXGISwapChain1> swap_chain;
        result =
            m_factory->CreateSwapChainForHwnd(m_command_queue.Get(), (HWND) native_window_handle,
                                              &desc, nullptr, nullptr, &swap_chain);

        if (FAILED(result))
        {
            last_error = Error::create("Failed to create swap chain for window", Error_Bind_Window);
            return false;
        }

        swap_chain.As(&target.swap_chain);
    }

    // Set initial frame sync (is probably 0)
    target.sync.frame_index = target.swap_chain->GetCurrentBackBufferIndex();

    // TODO: support fullscreen transitions
    result = m_factory->MakeWindowAssociation((HWND) native_window_handle, DXGI_MWA_NO_ALT_ENTER);

    if (FAILED(result))
    {
        last_error = Error::create("Failed to set alt-enter option on window", Error_Bind_Window);
        return false;
    }

    // descriptor heap
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
        desc.NumDescriptors = BUFFER_COUNT;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        result = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&target.rtv_heap));

        if (FAILED(result))
        {
            last_error =
                Error::create("Failed to create render target view heap", Error_Bind_Window);
            return false;
        }

        target.rtv_descriptor_size =
            m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // frame resources
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle(
            target.rtv_heap->GetCPUDescriptorHandleForHeapStart());

        for (UINT index = 0; index < BUFFER_COUNT; ++index)
        {
            result =
                target.swap_chain->GetBuffer(index, IID_PPV_ARGS(&target.render_targets[index]));
            if (FAILED(result))
            {
                last_error = Error::create("Failed to initialize render target resources",
                                           Error_Bind_Window);
                return false;
            }

            m_device->CreateRenderTargetView(target.render_targets[index].Get(), nullptr,
                                             rtv_handle);

            // They want devs to download a 350KB helper library for this one line of code?
            // Linus, say something horrible about Microsoft.
            rtv_handle.ptr += target.rtv_descriptor_size;
        }
    }

    m_window_targets[(HWND) native_window_handle] = std::move(target);

    // TODO: set whatever needs to be set

    return true;
}

bool DX12RenderEngine::render()
{

    return false;
}

void DX12RenderEngine::clear_commands()
{
}

void DX12RenderEngine::initialize()
{
    UINT dxgi_flags = 0;
    HRESULT result;

#ifdef DEBUG
    result = D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug_controller));
    if (SUCCEEDED(result))
    {
        m_debug_controller->EnableDebugLayer();
        dxgi_flags |= DXGI_CREATE_FACTORY_DEBUG;
    }
    else
    {
        last_error = Error::create("Failed to enable debug layer", Error_Init_Failed_Generic);
        return;
    }
#endif

    result = CreateDXGIFactory2(dxgi_flags, IID_PPV_ARGS(&m_factory));
    if (FAILED(result))
    {
        last_error = Error::create("Failed to create DXGI factory", Error_Init_Failed_Generic);
        return;
    }

    ptr<IDXGIAdapter4> adapter;
    GetHardwareAdapter(m_factory.Get(), &adapter, true);
    if (adapter.Get() == nullptr)
    {
        last_error = Error::create("Failed to get Direct3D 12 adapter", Error_Init_Failed_Generic);
        return;
    }

    result = D3D12CreateDevice(adapter.Get(), FEATURE_LEVEL, IID_PPV_ARGS(&m_device));
    if (FAILED(result))
    {
        last_error = Error::create("Failed to get Direct3D 12 device", Error_Init_Failed_Generic);
        return;
    }

    D3D12_COMMAND_QUEUE_DESC queue_desc{};
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    result = m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_command_queue));
    if (FAILED(result))
    {
        last_error =
            Error::create("Failed to create Direct3D command queue", Error_Init_Failed_Generic);
        return;
    }

    result = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                              IID_PPV_ARGS(&m_command_alloc));
    if (FAILED(result))
    {
        last_error =
            Error::create("Failed to create Direct3D command allocator", Error_Init_Failed_Generic);
        return;
    }

    // Create root signature
    D3D12_ROOT_SIGNATURE_DESC root_desc{};
    // yet another single line of code that demanded the 350KB helper library
    root_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ptr<ID3DBlob> signature;
    ptr<ID3DBlob> error;
    result =
        D3D12SerializeRootSignature(&root_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    if (FAILED(result))
    {
        last_error =
            Error::create("Failed to serialize Direct3D root signature", Error_Init_Failed_Generic);
        return;
    }

    result = m_device->CreateRootSignature(0, signature->GetBufferPointer(),
                                           signature->GetBufferSize(), IID_PPV_ARGS(&m_root));
    if (FAILED(result))
    {
        last_error =
            Error::create("Failed to create Direct3D root signature", Error_Init_Failed_Generic);
        return;
    }
}

const Error& DX12RenderEngine::get_last_error() const
{
    return last_error;
}

void DX12RenderEngine::debug_report_objects()
{
#ifdef DEBUG
    ptr<IDXGIDebug1> debug_check;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug_check));

    UINT flags = 0;
    flags |= DXGI_DEBUG_RLO_ALL;
    /*
    flags |= DXGI_DEBUG_RLO_DETAIL;
    flags |= DXGI_DEBUG_RLO_IGNORE_INTERNAL;
    */

    debug_check->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS) flags);
#endif
}

} // namespace Surface::Graphics

#endif
