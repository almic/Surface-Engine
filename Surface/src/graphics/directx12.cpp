#include "directx12.h"

// Windows only
#ifdef PLATFORM_WINDOWS

// Feature level
static inline constexpr D3D_FEATURE_LEVEL FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_0;

// Because using shared_ptr wasn't "Microsoft" enough, they require CamelCase!
template <typename T> using ptr = Microsoft::WRL::ComPtr<T>;

// Interface types, because the names are stupid and long
using Adapter = IDXGIAdapter4;
using Device = ID3D12Device10;
using Factory = IDXGIFactory7;
using SwapChain = IDXGISwapChain4;
using Resource = ID3D12Resource1;

// Lifted from:
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/UWP/D3D12HelloWorld/src/HelloTriangle/DXSample.cpp
static void GetHardwareAdapter(Factory* factory, Adapter** adapter_ptr, bool high_performance)
{

    *adapter_ptr = nullptr;
    SIZE_T memory = 0;
    ptr<Adapter> adapter;
    ptr<Adapter> best;
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
                D3D12CreateDevice(adapter.Get(), FEATURE_LEVEL, __uuidof(ID3D12Device), nullptr)) &&
            desc.DedicatedVideoMemory > memory)
        {
            adapter.As(&best);
            memory = desc.DedicatedVideoMemory;
        }
    }

    if (best.Get() == nullptr)
    {
        // Fallback to first supported adapter
        ptr<IDXGIAdapter1> adapter1;
        for (UINT index = 0; SUCCEEDED(factory->EnumAdapters1(index, &adapter1)); ++index)
        {
            DXGI_ADAPTER_DESC1 desc;
            if (FAILED(adapter1->GetDesc1(&desc)))
            {
                continue;
            }

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select software adapters
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(adapter1.Get(), FEATURE_LEVEL, __uuidof(ID3D12Device),
                                            nullptr)) &&
                desc.DedicatedVideoMemory > memory)
            {
                adapter1.As(&best);
                memory = desc.DedicatedVideoMemory;
            }
        }
    }

    *adapter_ptr = best.Detach();
}

namespace Barrier
{

static inline D3D12_RESOURCE_BARRIER
transition(const ptr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES before,
           D3D12_RESOURCE_STATES after,
           D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
           UINT subresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
{

    return {.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = flags,
            .Transition = {
                .pResource = resource.Get(),
                .Subresource = subresources,
                .StateBefore = before,
                .StateAfter = after,
            }};
}

} // namespace Barrier

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
        m_target = &m_window_targets.at((HWND) native_window_handle);
        return true;
    }

    // Set to nullptr until it is created successfully
    m_target = nullptr;
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
        desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

        ptr<IDXGISwapChain1> swap_chain1;
        result =
            m_factory->CreateSwapChainForHwnd(m_command_queue.Get(), (HWND) native_window_handle,
                                              &desc, nullptr, nullptr, &swap_chain1);

        if (FAILED(result))
        {
            last_error = Error::create("Failed to create swap chain for window", Error_Bind_Window);
            return false;
        }

        swap_chain1.As(&target.swap_chain);
    }

    // Set initial frame sync (is probably 0)
    target.sync.frame_index = target.swap_chain->GetCurrentBackBufferIndex();

    // Create sync resources
    {
        target.sync.fence_value = 0;
        result = m_device->CreateFence(target.sync.fence_value, D3D12_FENCE_FLAG_NONE,
                                       IID_PPV_ARGS(&target.sync.fence));

        if (FAILED(result))
        {
            last_error = Error::create("Failed to create fence for window", Error_Bind_Window);
            return false;
        }

        target.sync.fence_event = CreateEventA(0, 0, 0, 0);

        if (!target.sync.fence_event)
        {
            last_error =
                Error::create("Failed to create fence event for window", Error_Bind_Window);
            return false;
        }
    }

    // TODO: support fullscreen transitions
    result = m_factory->MakeWindowAssociation((HWND) native_window_handle, DXGI_MWA_NO_ALT_ENTER);

    if (FAILED(result))
    {
        last_error = Error::create("Failed to set alt-enter option on window", Error_Bind_Window);
        return false;
    }

    // render target descriptor heap
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
        desc.NumDescriptors = BUFFER_COUNT;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        result = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&target.rtv_heap.heap));

        if (FAILED(result))
        {
            last_error =
                Error::create("Failed to create render target view heap", Error_Bind_Window);
            return false;
        }

        target.rtv_heap.offset =
            m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // render target frame resources
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle(
            target.rtv_heap.heap->GetCPUDescriptorHandleForHeapStart());

        for (UINT index = 0; index < BUFFER_COUNT; ++index)
        {
            result = target.swap_chain->GetBuffer(index,
                                                  IID_PPV_ARGS(&target.rtv_heap.resources[index]));
            if (FAILED(result))
            {
                last_error = Error::create("Failed to initialize render target resources",
                                           Error_Bind_Window);
                return false;
            }

            m_device->CreateRenderTargetView(target.rtv_heap.resources[index].Get(), nullptr,
                                             rtv_handle);

            // They want devs to download a 350KB helper library for this one line of code?
            // Linus, say something horrible about Microsoft.
            rtv_handle.ptr += target.rtv_heap.offset;
        }
    }

    m_window_targets[(HWND) native_window_handle] = std::move(target);

    m_target = &m_window_targets.at((HWND) native_window_handle);

    return true;
}

bool DX12RenderEngine::render()
{
    if (!m_target)
    {
        return false;
    }

    HRESULT result;

    result = m_command_alloc->Reset();
    if (FAILED(result))
    {
        last_error = Error::create("Failed to reset command allocator", Error_Generic);
        return false;
    }

    result = m_command_list->Reset(m_command_alloc.Get(), nullptr);
    if (FAILED(result))
    {
        last_error = Error::create("Failed to reset command list", Error_Generic);
        return false;
    }

    auto frame_index = m_target->sync.frame_index;
    auto& buffer = m_target->rtv_heap.resources[frame_index];

    // Clear color
    {
        D3D12_RESOURCE_BARRIER barrier = Barrier::transition(buffer, D3D12_RESOURCE_STATE_PRESENT,
                                                             D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_command_list->ResourceBarrier(1, &barrier);

        float clear_color[] = {0.4f, 0.6f, 0.9f, 1.0f};
        D3D12_CPU_DESCRIPTOR_HANDLE rtv(
            m_target->rtv_heap.heap->GetCPUDescriptorHandleForHeapStart());

        // Another single line of code saved with a 350KB library!
        rtv.ptr += static_cast<SIZE_T>(frame_index) * m_target->rtv_heap.offset;

        m_command_list->ClearRenderTargetView(rtv, clear_color, 0, nullptr);
    }

    // Present
    {
        D3D12_RESOURCE_BARRIER barrier = Barrier::transition(
            buffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_command_list->ResourceBarrier(1, &barrier);

        result = m_command_list->Close();
        if (FAILED(result))
        {
            last_error = Error::create("Failed to close command list for frame", Error_Generic);
            return false;
        }

        ID3D12CommandList* const list[] = {
            m_command_list.Get(),
        };
        m_command_queue->ExecuteCommandLists(_countof(list), list);

        result = m_target->swap_chain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
        if (FAILED(result))
        {
            last_error = Error::create("Failed to present frame", Error_Generic);
            return false;
        }

        // Update frame and fence value
        m_target->sync.frame_index = m_target->swap_chain->GetCurrentBackBufferIndex();
        ++(m_target->sync.fence_value);

        // Signal fence value
        result = m_command_queue->Signal(m_target->sync.fence.Get(), m_target->sync.fence_value);
        if (FAILED(result))
        {
            last_error = Error::create("Failed to signal fence", Error_Generic);
            return false;
        }

        // TODO: remove this and create a better system to sync with GPU, such as just returning
        // from this method immediately if the fence value isn't correct

        // Wait for frame to be presented
        {
            if (m_target->sync.fence->GetCompletedValue() < m_target->sync.fence_value)
            {
                result = m_target->sync.fence->SetEventOnCompletion(m_target->sync.fence_value,
                                                                    m_target->sync.fence_event);
                WaitForSingleObject(m_target->sync.fence_event, INFINITE);
            }
        }
    }

    return true;
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

    ptr<Adapter> adapter;
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

#ifdef DEBUG
    ptr<ID3D12InfoQueue> info_queue;
    if (SUCCEEDED(m_device.As(&info_queue)))
    {
        // Debugger breakpoints
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, 1);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, 1);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, 1);

        D3D12_INFO_QUEUE_FILTER filter{};

        // Info messages can be suppressed
        D3D12_MESSAGE_SEVERITY ignored_severity[] = {
            D3D12_MESSAGE_SEVERITY_INFO,
        };

        filter.DenyList.pSeverityList = ignored_severity;
        filter.DenyList.NumSeverities = _countof(ignored_severity);

        D3D12_MESSAGE_ID ignored_messages[] = {
            // This happens when using custom clear colors
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,

            // These are caused by Visual Studio when using the frame capture debugger
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
        };

        filter.DenyList.pIDList = ignored_messages;
        filter.DenyList.NumIDs = _countof(ignored_messages);

        // Require this to work in debug
        result = info_queue->PushStorageFilter(&filter);
        if (FAILED(result))
        {
            last_error = Error::create("Failed to set debugger filters", Error_Init_Failed_Generic);
            return;
        }
    }
#endif

    // Create command queue
    D3D12_COMMAND_QUEUE_DESC queue_desc{};
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

    result = m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_command_queue));
    if (FAILED(result))
    {
        last_error =
            Error::create("Failed to create Direct3D command queue", Error_Init_Failed_Generic);
        return;
    }

    // Create command allocator
    result = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                              IID_PPV_ARGS(&m_command_alloc));
    if (FAILED(result))
    {
        last_error =
            Error::create("Failed to create Direct3D command allocator", Error_Init_Failed_Generic);
        return;
    }

    // Create command list
    result =
        m_device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_command_list));

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
