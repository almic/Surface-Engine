#pragma once

#include "graphics.h"

// Things for rendering with DirectX 12, Windows only
#ifdef PLATFORM_WINDOWS

// windows
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <wrl.h>

// dx12
#include <d3d12.h>
#include <dxgi1_6.h>

// dx12 debug
#ifdef DEBUG
#include <dxgidebug.h>
#endif

// stl
#include <unordered_map> // for managing swapchains

namespace Surface::Graphics
{

struct DX12RenderEngine : public RenderEngine
{

    DX12RenderEngine();

    template <typename T> using ptr = Microsoft::WRL::ComPtr<T>;

    // Latest types
    using Adapter = IDXGIAdapter4;
    using Device = ID3D12Device8;
    using Factory = IDXGIFactory7;
    using SwapChain = IDXGISwapChain4;
    using Resource = ID3D12Resource1;

  public: // Overrides
    bool bind_window(void* native_window_handle) override;
    bool render() override;
    void clear_commands() override;
    void initialize() override;
    const Error& get_last_error() const override;

  public: // Methods
    inline ID3D12Debug* get_debug() const
    {
#ifdef DEBUG
        return m_debug_controller.Get();
#else
        return nullptr;
#endif
    }

  public: // static methods
    static void debug_report_objects();

  public: // members
    // 2 buffers in the swap chain
    static inline constexpr UINT BUFFER_COUNT = 2;

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissor_rect;

#ifdef DEBUG
    ptr<ID3D12Debug> m_debug_controller;
#endif

    ptr<Factory> m_factory;
    ptr<ID3D12CommandAllocator> m_command_alloc;
    ptr<ID3D12GraphicsCommandList> m_command_list;
    ptr<ID3D12CommandQueue> m_command_queue;

    ptr<Adapter> m_adapter;
    ptr<Device> m_device;
    ptr<ID3D12RootSignature> m_root;
    ptr<ID3D12PipelineState> m_state;

    // Dynamically sized heap
    struct ResourceHeap
    {
        ptr<ID3D12DescriptorHeap> heap;
        ptr<Resource>* resources;
        UINT offset;
        size_t count;
    };

    // Statically sized heap
    template <size_t size> struct StaticResourceHeap
    {
        static constexpr size_t count = size;
        ptr<ID3D12DescriptorHeap> heap;
        ptr<Resource> resources[size];
        UINT offset = 0;
    };

    // A render target with its own heap, swap chain, and synchronization
    struct RenderTarget
    {
        ptr<SwapChain> swap_chain;

        StaticResourceHeap<BUFFER_COUNT> rtv_heap;

        // Synchronization
        struct Sync
        {
            UINT frame_index = 0;

            ptr<ID3D12Fence> fence;
            HANDLE fence_event = nullptr;
            uint64_t fence_value = 0;
        } sync;
    };

    std::unordered_map<HWND, RenderTarget> m_window_targets;

    RenderTarget* m_target;

    // Dev stuff
    ptr<ID3D12Resource> m_vertex_buffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertex_buffer_view;
};

} // namespace Surface::Graphics

#endif
