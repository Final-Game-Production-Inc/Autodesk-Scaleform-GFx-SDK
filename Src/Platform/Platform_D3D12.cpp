/**************************************************************************

Filename    :   Platform_D3D12.cpp
Content     :   Platform Device Support for D3D12.
Created     :   Mar 2026
Authors     :   FINALZ-27980

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/D3D12/D3D12_HAL.h"
#include "Render/Render_DebugEnv.h"
#include "Platform.h"
#include <WinReg.h>
#include <ShlObj.h>

#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Kernel/SF_SysFile.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Threads.h"

#include <dxgi.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcommon.h>
#include <d3d12sdklayers.h>
#include <wrl/client.h>

namespace Scaleform { namespace Platform {

using Microsoft::WRL::ComPtr;

static void DumpD3D12DebugMessages(ID3D12Device* pDevice, const char* pTag = 0)
{
    if (!pDevice)
        return;
    ID3D12InfoQueue* pQueue = 0;
    if (FAILED(pDevice->QueryInterface(IID_PPV_ARGS(&pQueue))) || !pQueue)
        return;

    UINT64 count = pQueue->GetNumStoredMessages();
    for (UINT64 i = 0; i < count; i++)
    {
        SIZE_T length = 0;
        if (FAILED(pQueue->GetMessage(i, 0, &length)) || !length)
            continue;
        D3D12_MESSAGE* pMessage = (D3D12_MESSAGE*)malloc(length);
        if (!pMessage)
            continue;
        if (SUCCEEDED(pQueue->GetMessage(i, pMessage, &length)))
        {
            // The message ID is what makes a dump actionable - the description
            // text of several distinct errors is identical.
            fprintf(stderr, "[D3D12-DEBUG%s] id=%d sev=%d %s\n",
                    pTag ? pTag : "", (int)pMessage->ID, (int)pMessage->Severity,
                    pMessage->pDescription);
            fflush(stderr);
        }
        free(pMessage);
    }
    pQueue->ClearStoredMessages();
    pQueue->Release();
}

class DeviceImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:
    DeviceImpl(Render::ThreadCommandQueue *commandQueue);
    ~DeviceImpl();

    void      calcSwapChainParams(DXGI_SWAP_CHAIN_DESC1* params, const ViewConfig& config);
    bool      adjustViewConfig(ViewConfig* config);

    bool initGraphics(const ViewConfig& config, Device::Window* window, ThreadId renderThreadId = 0);
    bool createSwapChain(const ViewConfig& config);
    void destroySwapChain(bool finalTeardown = false);
    bool createDepthStencil(const DXGI_SWAP_CHAIN_DESC1& desc);
    bool createDepthStencilFromSize(UINT width, UINT height);
    void destroyDepthStencil();
    bool reconfigureGraphics(const ViewConfig& config);
    void shutdownGraphics();

    void beginFrame();
    void presentFrame();
    bool takeScreenShot(const String& filename);

    void updateStatus();

    Device::Window*             pWindow;
    Ptr<Render::D3D12::HAL>     pHal;
    DeviceStatus                Status;

    HWND                            hWnd;
    ComPtr<IDXGIFactory4>           pDXGIFactory;
    ComPtr<IDXGISwapChain3>         pSwapChain;
    ComPtr<ID3D12Device>            pDevice;
    ComPtr<ID3D12CommandQueue>      pCommandQueue;
    ComPtr<ID3D12CommandAllocator>  pCommandAllocator;
    ComPtr<ID3D12GraphicsCommandList> pCommandList;

    // Descriptor heaps.
    ComPtr<ID3D12DescriptorHeap>    pRTVHeap;       // Swapchain back buffer views.
    ComPtr<ID3D12DescriptorHeap>    pDSVHeap;       // Depth/stencil view.
    ComPtr<ID3D12DescriptorHeap>    pSRVHeap;       // CPU-only CBV/SRV/UAV heap (HALInitParams).

    // Swapchain buffers.
    static const unsigned           MaxSwapBuffers = 4;
    ComPtr<ID3D12Resource>          pSwapBuffers[MaxSwapBuffers];
    D3D12_CPU_DESCRIPTOR_HANDLE     RTVHandles[MaxSwapBuffers];
    UINT                            SwapBufferCount;

    // Depth/stencil buffer.
    ComPtr<ID3D12Resource>          pDepthStencil;
    D3D12_CPU_DESCRIPTOR_HANDLE     DSVHandle;

    DXGI_FORMAT                     SwapChainFormat;
    DXGI_FORMAT                     DepthStencilFormat;
    UINT                            SwapChainWidth;
    UINT                            SwapChainHeight;

    // Frame synchronization.
    ComPtr<ID3D12Fence>             pFrameFence;
    HANDLE                          FrameFenceEvent;
    UINT64                          FrameFenceValue;

    Scaleform::ThreadId             RenderThreadId;
    bool                            VSync;
};

DeviceImpl::DeviceImpl(Render::ThreadCommandQueue *commandQueue)
 : pWindow(0), pHal(0), Status(Device_NeedInit),
   hWnd(0),
   SwapBufferCount(0),
   SwapChainFormat(DXGI_FORMAT_B8G8R8A8_UNORM),
   DepthStencilFormat(DXGI_FORMAT_D24_UNORM_S8_UINT),
   SwapChainWidth(0), SwapChainHeight(0),
   FrameFenceEvent(0), FrameFenceValue(0),
   RenderThreadId(0),
   VSync(false)
{
    pHal = *SF_NEW Render::D3D12::HAL(commandQueue);
}

DeviceImpl::~DeviceImpl()
{
    destroySwapChain();
    destroyDepthStencil();
    pHal.Clear();
    if (FrameFenceEvent)
        CloseHandle(FrameFenceEvent);
}

void DeviceImpl::calcSwapChainParams(DXGI_SWAP_CHAIN_DESC1* params, const ViewConfig& config)
{
    DXGI_SWAP_CHAIN_DESC1 desc;
    memset(&desc, 0, sizeof(desc));

    VSync = config.HasFlag(View_VSync);

    desc.Width              = config.ViewSize.Width;
    desc.Height             = config.ViewSize.Height;
    desc.Format             = SwapChainFormat;
    desc.Stereo             = FALSE;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount        = 2;
    desc.Scaling            = DXGI_SCALING_STRETCH;
    desc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
    // Keep a waitable object so shutdown can wait until the last presented
    // frame has been retired by DXGI before the back buffers are released.
    desc.Flags              = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

    *params = desc;
}

bool DeviceImpl::adjustViewConfig(ViewConfig* config)
{
    // D3D12 does not support stereo.
    config->SetFlag(View_Stereo, false);

    if (config->HasFlag(View_Size))
    {
        if (config->ViewSize.Width < 1)
            config->ViewSize.Width = 1;
        if (config->ViewSize.Height < 1)
            config->ViewSize.Height = 1;
    }
    else
    {
        Size<unsigned> size = Device::GetDefaultViewSize();
        config->SetSize(size.Width, size.Height);
    }

    if (config->HasFlag(View_FullScreen))
    {
        if (!config->HasFlag(View_Pos))
            config->SetPos(0, 0);
    }
    return true;
}

bool DeviceImpl::initGraphics(const ViewConfig& config, Device::Window* window,
                              ThreadId renderThreadId)
{
    if (!pHal || !window)
        return false;

    pWindow         = window;
    hWnd            = (HWND)window->GetHandle();
    RenderThreadId  = renderThreadId;

    // Create the DXGI factory.
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&pDXGIFactory))))
        return false;

    // Create the D3D12 device.
#if defined(SF_BUILD_DEBUG)
    ID3D12Debug* pDebug = 0;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug))) && pDebug)
    {
        pDebug->EnableDebugLayer();
        pDebug->Release();
    }
#endif
    HRESULT hr = D3D12CreateDevice(0, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice));
    if (FAILED(hr))
    {
        SF_DEBUG_WARNING1(1, "Failed to create D3D12 device (hr=0x%08x). "
            "D3D12 requires Windows 10 and a DirectX 12 capable GPU.", (unsigned)hr);
        return false;
    }

    // Command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc;
    memset(&queueDesc, 0, sizeof(queueDesc));
    queueDesc.Type     = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;
    if (FAILED(pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&pCommandQueue))))
        return false;

    // Command allocator + command list. The list is left OPEN on purpose:
    // HAL::InitHAL records the initial render-target setup (OMSetRenderTargets,
    // clear, viewport) while creating the default render buffer, and D3D12 fails
    // every recording call on a closed list ("This API cannot be called on a
    // closed command list"). The platform closes it again in presentFrame().
    if (FAILED(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                               IID_PPV_ARGS(&pCommandAllocator))))
        return false;
    if (FAILED(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                          pCommandAllocator.Get(), 0,
                                          IID_PPV_ARGS(&pCommandList))))
        return false;

    // CPU-only descriptor heap for the HAL (kept for parity; the TextureManager
    // creates its own GPU-visible heaps).
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    memset(&heapDesc, 0, sizeof(heapDesc));
    heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = 64;
    heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pSRVHeap))))
        return false;

    // Frame fence.
    if (FAILED(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFrameFence))))
        return false;
    FrameFenceEvent = CreateEvent(0, FALSE, FALSE, 0);
    if (!FrameFenceEvent)
        return false;
    FrameFenceValue = 0;

    if (!createSwapChain(config))
        return false;

    // Initialize the HAL (which creates the default render buffer from MainRT).
    Render::D3D12::HALInitParams params(pDevice.Get(), pCommandQueue.Get(),
                                        pCommandAllocator.Get(), pCommandList.Get(),
                                        pSRVHeap.Get(), 0, renderThreadId);
    if (!pHal->InitHAL(params))
    {
        shutdownGraphics();
        pWindow = 0;
        hWnd = 0;
        return false;
    }

    // InitHAL recorded the default render-target setup into the list; close it so
    // the first beginFrame() can reset the allocator and the list (D3D12 refuses
    // to reset an allocator while a list is still recording, and refuses to reset
    // a list that was never closed).
    pCommandList->Close();

    Status = Device_Ready;
    return true;
}

bool DeviceImpl::createSwapChain(const ViewConfig& config)
{
    if (!pDXGIFactory || !pDevice)
        return false;

    destroySwapChain();

    DXGI_SWAP_CHAIN_DESC1 desc;
    calcSwapChainParams(&desc, config);
    SwapChainWidth  = desc.Width;
    SwapChainHeight = desc.Height;

    ComPtr<IDXGISwapChain1> pswapChain;
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc;
    memset(&fsDesc, 0, sizeof(fsDesc));
    fsDesc.Windowed = !config.HasFlag(View_FullScreen);

    HRESULT hr = pDXGIFactory->CreateSwapChainForHwnd(pCommandQueue.Get(), hWnd,
                                                      &desc, &fsDesc, 0, &pswapChain);
    if (FAILED(hr))
    {
        SF_DEBUG_WARNING1(1, "Failed to create D3D12 swap chain (hr=0x%08x).", (unsigned)hr);
        return false;
    }
    pSwapChain = static_cast<IDXGISwapChain3*>(pswapChain.Get());
    pSwapChain->SetMaximumFrameLatency(1);
    pDXGIFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

    // Create RTVs for the swapchain buffers.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    memset(&rtvHeapDesc, 0, sizeof(rtvHeapDesc));
    rtvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = MaxSwapBuffers;
    rtvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&pRTVHeap))))
        return false;

    UINT rtvIncrement = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pRTVHeap->GetCPUDescriptorHandleForHeapStart();

    SwapBufferCount = desc.BufferCount;
    for (UINT i = 0; i < SwapBufferCount; i++)
    {
        if (FAILED(pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pSwapBuffers[i]))))
            return false;
        // Debug names make debug-layer messages ("resource X is referenced by
        // GPU operations in-flight", leaks, ...) readable with '-debug'.
        wchar_t bufname[32];
        swprintf_s(bufname, L"GFx_SwapBuffer%u", i);
        pSwapBuffers[i]->SetName(bufname);
        pDevice->CreateRenderTargetView(pSwapBuffers[i].Get(), 0, rtvHandle);
        RTVHandles[i] = rtvHandle;
        rtvHandle.ptr += rtvIncrement;
    }

    if (!createDepthStencil(desc))
        return false;

    // Attach the initial back buffer to the HAL as the default render target.
    UINT currentIndex = pSwapChain->GetCurrentBackBufferIndex();
    pHal->SetMainRenderTarget(pSwapBuffers[currentIndex].Get(), RTVHandles[currentIndex],
                              SwapChainFormat, Render::ImageSize(SwapChainWidth, SwapChainHeight),
                              pDepthStencil.Get(), DSVHandle);
    return true;
}

bool DeviceImpl::createDepthStencil(const DXGI_SWAP_CHAIN_DESC1& desc)
{
    return createDepthStencilFromSize(desc.Width, desc.Height);
}

bool DeviceImpl::createDepthStencilFromSize(UINT width, UINT height)
{
    destroyDepthStencil();

    if (DepthStencilFormat == DXGI_FORMAT_UNKNOWN)
        return true;

    D3D12_HEAP_PROPERTIES heapProps;
    memset(&heapProps, 0, sizeof(heapProps));
    heapProps.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask     = 1;
    heapProps.VisibleNodeMask      = 1;

    D3D12_RESOURCE_DESC resDesc;
    memset(&resDesc, 0, sizeof(resDesc));
    resDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resDesc.Alignment          = 0;
    resDesc.Width              = width;
    resDesc.Height             = height;
    resDesc.DepthOrArraySize   = 1;
    resDesc.MipLevels          = 1;
    resDesc.Format             = DepthStencilFormat;
    resDesc.SampleDesc.Count   = 1;
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resDesc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format               = DepthStencilFormat;
    clearValue.DepthStencil.Depth   = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    if (FAILED(pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                                                &resDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                &clearValue, IID_PPV_ARGS(&pDepthStencil))))
        return false;

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    memset(&dsvHeapDesc, 0, sizeof(dsvHeapDesc));
    dsvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&pDSVHeap))))
        return false;

    pDepthStencil->SetName(L"GFx_DepthStencil");
    DSVHandle = pDSVHeap->GetCPUDescriptorHandleForHeapStart();
    pDevice->CreateDepthStencilView(pDepthStencil.Get(), 0, DSVHandle);
    return true;
}

void DeviceImpl::destroyDepthStencil()
{
    pDepthStencil = nullptr;
    pDSVHeap = nullptr;
    DSVHandle.ptr = 0;
}

bool DeviceImpl::reconfigureGraphics(const ViewConfig& config)
{
    if (!pDevice || !pSwapChain || !pHal)
        return false;

    // This function may be recursive, so only allow one entry.
    static bool reconfigureActive = false;
    if (reconfigureActive)
        return true;
    reconfigureActive = true;

    pHal->PrepareForReset();

    UINT newWidth  = config.ViewSize.Width;
    UINT newHeight = config.ViewSize.Height;

    // Wait for outstanding GPU work so the swapchain buffers can be released.
    if (FrameFenceValue > 0)
    {
        pCommandQueue->Signal(pFrameFence.Get(), ++FrameFenceValue);
        if (pFrameFence->GetCompletedValue() < FrameFenceValue)
        {
            pFrameFence->SetEventOnCompletion(FrameFenceValue, FrameFenceEvent);
            WaitForSingleObject(FrameFenceEvent, INFINITE);
        }
    }

    // Release all references to the back buffers.
    for (UINT i = 0; i < SwapBufferCount; i++)
        pSwapBuffers[i] = nullptr;
    pRTVHeap = nullptr;
    destroyDepthStencil();

    HRESULT resizeHr = pSwapChain->ResizeBuffers(2, newWidth, newHeight, SwapChainFormat,
                                                      DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
    if (FAILED(resizeHr))
    {
        reconfigureActive = false;
        return false;
    }
    SwapChainWidth  = newWidth;
    SwapChainHeight = newHeight;

    // Recreate the RTVs.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    memset(&rtvHeapDesc, 0, sizeof(rtvHeapDesc));
    rtvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = MaxSwapBuffers;
    rtvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&pRTVHeap))))
    {
        reconfigureActive = false;
        return false;
    }
    UINT rtvIncrement = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pRTVHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < SwapBufferCount; i++)
    {
        if (FAILED(pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pSwapBuffers[i]))))
        {
            reconfigureActive = false;
            return false;
        }
        pDevice->CreateRenderTargetView(pSwapBuffers[i].Get(), 0, rtvHandle);
        RTVHandles[i] = rtvHandle;
        rtvHandle.ptr += rtvIncrement;
    }

    if (!createDepthStencilFromSize(newWidth, newHeight))
    {
        reconfigureActive = false;
        return false;
    }

    // Re-attach the default render target and restore the HAL.
    UINT currentIndex = pSwapChain->GetCurrentBackBufferIndex();
    pHal->SetMainRenderTarget(pSwapBuffers[currentIndex].Get(), RTVHandles[currentIndex],
                              SwapChainFormat, Render::ImageSize(SwapChainWidth, SwapChainHeight),
                              pDepthStencil.Get(), DSVHandle);
    pHal->RestoreAfterReset();
    VSync = config.HasFlag(View_VSync);
    Status = Device_Ready;

    reconfigureActive = false;
    return true;
}

void DeviceImpl::shutdownGraphics()
{
    if (!pDevice)
        return;

    // Retire the last presented frame first (flip-model presents are not
    // tracked by our frame fence), otherwise releasing the back buffers below
    // trips the debug layer's final-release CORRUPTION check.
    if (pSwapChain)
    {
        HANDLE hLatencyWait = pSwapChain->GetFrameLatencyWaitableObject();
        if (hLatencyWait)
            WaitForSingleObject(hLatencyWait, 1000);
    }

    // Drain the GPU and drop anything the (unsubmitted or submitted) command
    // list still references. Releasing a resource that a recorded command list
    // points at is invalid and makes the D3D12 debug layer raise
    // (0x87D) during the release below - that was the crash on exit.
    if (pCommandQueue && pFrameFence && FrameFenceValue > 0 &&
        pFrameFence->GetCompletedValue() < FrameFenceValue)
    {
        pFrameFence->SetEventOnCompletion(FrameFenceValue, FrameFenceEvent);
        WaitForSingleObject(FrameFenceEvent, INFINITE);
    }
    if (pCommandList && pCommandAllocator)
    {
        pCommandAllocator->Reset();
        pCommandList->Reset(pCommandAllocator.Get(), 0);
        pCommandList->Close();
    }

    // Tear down the HAL while the swap-chain resources are still alive: the HAL
    // releases its own references to them (MainRT and the default render
    // target), so they must not be destroyed first.
    if (pHal)
        pHal->ShutdownHAL();

    destroySwapChain(true);
    destroyDepthStencil();

    pCommandList = nullptr;
    pCommandAllocator = nullptr;
    pCommandQueue = nullptr;
    pSRVHeap = nullptr;
    pFrameFence = nullptr;
    if (FrameFenceEvent)
    {
        CloseHandle(FrameFenceEvent);
        FrameFenceEvent = 0;
    }
    FrameFenceValue = 0;
    pDevice = nullptr;
    pDXGIFactory = nullptr;

    pWindow = 0;
    hWnd = 0;
    Status = Device_NeedInit;
}

void DeviceImpl::destroySwapChain(bool finalTeardown)
{
    if (finalTeardown)
    {
        // Final process teardown: hand the swap chain and its back buffers over
        // to DXGI without releasing our references. Releasing them makes the
        // D3D12 debug layer report
        //   "ID3D12Resource::<final-release>: CORRUPTION ... resource is
        //    referenced by GPU operations in-flight on Command Queue"
        // and raise a fatal 0x87D exception: for a flip-model swap chain the
        // runtime still tracks the last presented buffer as in flight even after
        // the GPU fence has been waited on. The process is about to exit, so the
        // references go with it; render-target recreation (resize) still uses
        // the normal release path below.
        pSwapChain.Detach();
        pRTVHeap.Detach();
        for (UINT i = 0; i < SwapBufferCount; i++)
            pSwapBuffers[i].Detach();
        SwapBufferCount = 0;
        SwapChainWidth = 0;
        SwapChainHeight = 0;
        return;
    }
    // Release the swap chain FIRST. For a flip-model swap chain DXGI keeps the
    // most recently presented buffer in flight; destroying the swap chain makes
    // DXGI retire those presents and drop its own buffer references. Releasing
    // our references first made the app drop the *last* reference to a back
    // buffer while it was still referenced by in-flight present work, which the
    // debug layer reports as
    //   "ID3D12Resource::<final-release>: CORRUPTION ... in-flight on Command
    //    Queue" and turns into a fatal 0x87D exception on shutdown.
    pSwapChain = nullptr;

    // The RTV heap also references the buffers; drop it before our own refs.
    pRTVHeap = nullptr;

    for (UINT i = 0; i < SwapBufferCount; i++)
        pSwapBuffers[i] = nullptr;
    SwapBufferCount = 0;
    SwapChainWidth = 0;
    SwapChainHeight = 0;
}

void DeviceImpl::beginFrame()
{
    if (!pCommandList || !pCommandAllocator)
        return;

    // A single allocator/list pair is reused every frame, so the GPU must be
    // done with the previously submitted list before the allocator is reset.
    // FrameFenceValue holds the value signalled after the last
    // ExecuteCommandLists call; waiting on it guarantees that. (The old check
    // "FrameFenceValue > 1" skipped the wait on the second frame, so the
    // allocator was reset while frame 1 was still executing -
    // "The command allocator was reset after the command list was recorded".)
    if (FrameFenceValue > 0)
    {
        LARGE_INTEGER t0, t1, freq;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&t0);
        if (pFrameFence->GetCompletedValue() < FrameFenceValue)
        {
            pFrameFence->SetEventOnCompletion(FrameFenceValue, FrameFenceEvent);
            WaitForSingleObject(FrameFenceEvent, INFINITE);
        }
        QueryPerformanceCounter(&t1);
        if (Render::IsDebugEnvSet("GFX_D3D12_FRAMETIME"))
        {
            const double ms = 1000.0 * double(t1.QuadPart - t0.QuadPart) / double(freq.QuadPart);
            if (ms > 100.0)
            {
                fprintf(stderr, "[FT] gpu-wait %.1f ms (frame %llu)\n", ms,
                        (unsigned long long)FrameFenceValue);
                fflush(stderr);
            }
        }
    }

    pCommandAllocator->Reset();
    pCommandList->Reset(pCommandAllocator.Get(), 0);

    // Update the HAL's default render target to the current back buffer.
    UINT currentIndex = pSwapChain->GetCurrentBackBufferIndex();
    pHal->SetMainRenderTarget(pSwapBuffers[currentIndex].Get(), RTVHandles[currentIndex],
                              SwapChainFormat, Render::ImageSize(SwapChainWidth, SwapChainHeight),
                              pDepthStencil.Get(), DSVHandle);
}

void DeviceImpl::presentFrame()
{
    if (!pCommandList || !pSwapChain)
        return;

    // Close and execute the command list recorded by the HAL this frame.
    if (FAILED(pCommandList->Close()))
        return;

    // Debug aid: dump the messages the validation layer produced while the
    // frame was recorded, before Present can fail/Crash. Set GFX_D3D12_DUMPMSG=1.
    if (Render::IsDebugEnvSet("GFX_D3D12_DUMPMSG"))
        DumpD3D12DebugMessages(pDevice.Get(), "-PRE");

    ID3D12CommandList* ppCommandLists[] = { pCommandList.Get() };
    pCommandQueue->ExecuteCommandLists(1, ppCommandLists);

    // The HAL records fence values while the list is being built; now that the
    // work is actually queued, signal them so that IsPending() reflects real GPU
    // progress (see Render::D3D12::RenderSync::FlushPendingSignals).
    if (pHal)
    {
        Render::D3D12::RenderSync* psync =
            static_cast<Render::D3D12::RenderSync*>(pHal->GetRenderSync());
        if (psync)
            psync->FlushPendingSignals();
    }

    // Signal the frame fence after all work (the HAL also signals its own
    // render-sync fence on the same queue).
    pCommandQueue->Signal(pFrameFence.Get(), ++FrameFenceValue);

    HRESULT presentHr = pSwapChain->Present(VSync ? 1 : 0, 0);
    if (FAILED(presentHr))
    {
        HRESULT removed = pDevice ? pDevice->GetDeviceRemovedReason() : 0;
        fprintf(stderr, "[D3D12] Present FAILED frame=%llu hr=0x%08X removed=0x%08X\n",
                (unsigned long long)FrameFenceValue, (unsigned)presentHr, (unsigned)removed);
        fflush(stderr);
        DumpD3D12DebugMessages(pDevice.Get(), "-REMOVED");
        // Stop the render thread from recording into a dead device; otherwise
        // every further frame spams the debug layer and the app looks hung.
        Status = Device_Lost;
    }
}

void DeviceImpl::updateStatus()
{
    if (Status != Device_Ready || !pDevice)
        return;

    HRESULT removed = pDevice->GetDeviceRemovedReason();
    if (removed != S_OK)
    {
        fprintf(stderr, "[D3D12] Device removed (reason=0x%08X)\n", (unsigned)removed);
        fflush(stderr);
        DumpD3D12DebugMessages(pDevice.Get(), "-LOST");
        Status = Device_Lost;
    }
}

bool DeviceImpl::takeScreenShot(const String& filename)
{
    if (!pSwapChain || !pDevice || !pCommandQueue)
        return false;

    String actualFilename = filename;
    String relpath = filename.GetPath();
    TCHAR workingDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, workingDir);
    String path(workingDir);
    path += '/';
    path += relpath;
    StringBuffer sb;
    for (UPInt i = 0; i < path.GetLength(); ++i)
    {
        char c = path[i];
        if (c != '/')
            sb.AppendChar(c);
        else
            sb.AppendChar('\\');
    }
    path = sb;
    if (filename.GetExtension().CompareNoCase(".png") != 0)
        actualFilename.AppendString(".png");
    SHCreateDirectoryExA(0, path, NULL);

    UINT currentIndex = pSwapChain->GetCurrentBackBufferIndex();
    ComPtr<ID3D12Resource> pbackBuffer = pSwapBuffers[currentIndex];

    // Create a readback buffer.
    UINT width  = SwapChainWidth;
    UINT height = SwapChainHeight;
    UINT64 rowPitch = (width * 4 + 255) & ~255u;
    UINT64 bufferSize = rowPitch * height;

    D3D12_HEAP_PROPERTIES heapProps;
    memset(&heapProps, 0, sizeof(heapProps));
    heapProps.Type                 = D3D12_HEAP_TYPE_READBACK;
    heapProps.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask     = 1;
    heapProps.VisibleNodeMask      = 1;

    D3D12_RESOURCE_DESC resDesc;
    memset(&resDesc, 0, sizeof(resDesc));
    resDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Alignment          = 0;
    resDesc.Width              = (UINT64)bufferSize;
    resDesc.Height             = 1;
    resDesc.DepthOrArraySize   = 1;
    resDesc.MipLevels          = 1;
    resDesc.Format             = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count   = 1;
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    ComPtr<ID3D12Resource> pReadback;
    if (FAILED(pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                                                &resDesc, D3D12_RESOURCE_STATE_COPY_DEST,
                                                NULL, IID_PPV_ARGS(&pReadback))))
        return false;

    // Record the copy: transition the back buffer, copy, transition back.
    D3D12_RESOURCE_BARRIER barrier;
    memset(&barrier, 0, sizeof(barrier));
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = pbackBuffer.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_SOURCE;
    pCommandList->ResourceBarrier(1, &barrier);

    D3D12_TEXTURE_COPY_LOCATION src, dst;
    src.pResource = pbackBuffer.Get();
    src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.SubresourceIndex = 0;
    dst.pResource = pReadback.Get();
    dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    dst.PlacedFootprint.Offset = 0;
    dst.PlacedFootprint.Footprint.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    dst.PlacedFootprint.Footprint.Width  = width;
    dst.PlacedFootprint.Footprint.Height = height;
    dst.PlacedFootprint.Footprint.Depth  = 1;
    dst.PlacedFootprint.Footprint.RowPitch = (UINT)rowPitch;
    D3D12_BOX box = { 0, 0, 0, width, height, 1 };
    pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, &box);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    pCommandList->ResourceBarrier(1, &barrier);

    pCommandList->Close();
    ID3D12CommandList* ppCommandLists[] = { pCommandList.Get() };
    pCommandQueue->ExecuteCommandLists(1, ppCommandLists);
    pCommandQueue->Signal(pFrameFence.Get(), ++FrameFenceValue);
    if (pFrameFence->GetCompletedValue() < FrameFenceValue)
    {
        pFrameFence->SetEventOnCompletion(FrameFenceValue, FrameFenceEvent);
        WaitForSingleObject(FrameFenceEvent, INFINITE);
    }

    // Map and save.
    D3D12_RANGE range = { 0, (SIZE_T)bufferSize };
    UByte* pData = 0;
    if (FAILED(pReadback->Map(0, &range, (void**)&pData)))
        return false;

    // Convert BGRA to RGBA while saving.
    Render::ImageData data;
    UInt32* pSrcRow = (UInt32*)pData;
	SF_UNUSED(pSrcRow);
    UInt32* pDstRow = (UInt32*)SF_ALLOC(width * height * 4, Stat_Default_Mem);
    for (UINT y = 0; y < height; y++)
    {
        UInt32* ps = (UInt32*)((UByte*)pData + y * rowPitch);
        UInt32* pd = pDstRow + y * width;
        for (UINT x = 0; x < width; x++)
        {
            UInt32 c = ps[x];
            pd[x] = 0xFF000000 | ((c & 0x00FF0000) >> 16) | (c & 0x0000FF00) | ((c & 0x000000FF) << 16);
        }
    }
    data.Initialize(Render::Image_R8G8B8A8, width, height, width * 4, (UByte*)pDstRow);

#ifdef SF_ENABLE_LIBPNG
    bool success = Render::PNG::FileWriter::WriteImage<SysFile>(actualFilename, data);
#else
    bool success = false;
#endif
    SF_FREE(pDstRow);
    pReadback->Unmap(0, 0);

    // Reopen the command list for the next frame.
    pCommandAllocator->Reset();
    pCommandList->Reset(pCommandAllocator.Get(), 0);
    return success;
}

//------------------------------------------------------------------------
Device::Device(Render::ThreadCommandQueue *commandQueue)
{
    pImpl = SF_NEW DeviceImpl(commandQueue);
}

Device::~Device()
{
    delete pImpl;
}

Render::HAL* Device::GetHAL() const
{
    return pImpl->pHal;
}
DeviceStatus Device::GetStatus() const
{
    pImpl->updateStatus();
    return pImpl->Status;
}

//------------------------------------------------------------------------
bool Device::GraphicsConfigOnMainThread() const
{
    return false;
}

void Device::ResizeFrame(void*) { }

bool Device::AdjustViewConfig(ViewConfig* config)
{
    return pImpl->adjustViewConfig(config);
}

bool Device::InitGraphics(const ViewConfig& config, Device::Window* window,
                          ThreadId renderThreadId)
{
    return pImpl->initGraphics(config, window, renderThreadId);
}
bool Device::ReconfigureGraphics(const ViewConfig& config)
{
    return pImpl->reconfigureGraphics(config);
}
void Device::ShutdownGraphics()
{
    pImpl->shutdownGraphics();
}

Size<unsigned> Device::GetDefaultViewSize()
{
    return Size<unsigned>(1024, 768);
}

//------------------------------------------------------------------------
void Device::Clear(UInt32 color)
{
    if (!pImpl || !pImpl->pSwapChain || !pImpl->pCommandList)
        return;

    // D3D12 has no implicit clear; honor the app's background color here.
    float clear[4];
    clear[0] = ((color >> 16) & 0xFF) / 255.0f;
    clear[1] = ((color >> 8) & 0xFF) / 255.0f;
    clear[2] = (color & 0xFF) / 255.0f;
    clear[3] = 1.0f;

    UINT index = pImpl->pSwapChain->GetCurrentBackBufferIndex();
    if (index < pImpl->SwapBufferCount)
        pImpl->pCommandList->ClearRenderTargetView(pImpl->RTVHandles[index], clear, 0, 0);
}

int AppBase::GetDisplayCount()
{
    return 1;
}

Device::Window* Device::CreateGraphicsWindow(int, const ViewConfig&)
{
    return 0;
}

void Device::SetWindow(Window*)
{
}

void Device::PresentFrame(unsigned)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("Device::PresentFrame", Amp_Native_Function_Id_Present);
    pImpl->presentFrame();
}

void Device::BeginFrame()
{
    pImpl->beginFrame();
}

void Device::SetWireframe(bool flag)
{
    pImpl->pHal->SetRasterMode(flag ? Render::D3D12::HAL::RasterMode_Wireframe :
        Render::D3D12::HAL::RasterMode_Default);
}

UInt32 Device::GetCaps() const
{
    return DeviceCap_Wireframe;
}

bool Device::TakeScreenShot(const String& filename)
{
    return pImpl->takeScreenShot(filename);
}

}} // Scaleform::Platform
