/**************************************************************************

Filename    :   CaptureD3D11.cpp
Content     :   Headless D3D11 frame capture tool for visual regression testing.
                Renders a SWF to specified frames and saves PPM images.

CLI:    CaptureD3D11.exe <swf_path> <output_dir> --frames 1,10,30,60
                         [--width 1024] [--height 768]

**************************************************************************/

#define SF_D3D_VERSION 11

#include "GFx_Kernel.h"
#include "GFx.h"
#include "GFx_Renderer_D3D1x.h"
#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/ImageFiles/DDS_ImageFile.h"
#include "GFx/AS3/AS3_Global.h"
#include "GFx_FontProvider_Win32.h"
#include "Render/Renderer2D.h"

#include <windows.h>
#include <d3d11.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>

namespace SF = Scaleform;
using namespace Scaleform;
using namespace Render;
using namespace GFx;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void MakeDir(const char* path)
{
    CreateDirectoryA(path, NULL);
}

static void EnsureDirForFile(const char* filePath)
{
    char tmp[MAX_PATH];
    strncpy_s(tmp, filePath, MAX_PATH - 1);
    for (char* p = tmp; *p; p++)
        if (*p == '/') *p = '\\';
    char* last = strrchr(tmp, '\\');
    if (last)
    {
        *last = '\0';
        // Create all intermediate directories
        for (char* p = tmp + 1; *p; p++)
        {
            if (*p == '\\')
            {
                *p = '\0';
                CreateDirectoryA(tmp, NULL);
                *p = '\\';
            }
        }
        CreateDirectoryA(tmp, NULL);
    }
}

static bool WritePPM(const char* filename, const uint8_t* rgba, uint32_t w, uint32_t h, uint32_t rowPitch)
{
    FILE* f = fopen(filename, "wb");
    if (!f) return false;
    fprintf(f, "P6\n%u %u\n255\n", w, h);
    for (uint32_t row = 0; row < h; row++)
    {
        const uint8_t* px = rgba + row * rowPitch;
        for (uint32_t col = 0; col < w; col++, px += 4)
        {
            // D3D11 backbuffer is RGBA or BGRA depending on format.
            // SwapChain is DXGI_FORMAT_R8G8B8A8_UNORM — R=px[0], G=px[1], B=px[2].
            fputc(px[0], f); // R
            fputc(px[1], f); // G
            fputc(px[2], f); // B
        }
    }
    fclose(f);
    return true;
}

// ---------------------------------------------------------------------------
// Command queue (single-threaded: execute immediately)
// ---------------------------------------------------------------------------

class CaptureThreadCommandQueue : public ThreadCommandQueue
{
public:
    HAL*       pHAL;
    Renderer2D* pR2D;

    virtual void GetRenderInterfaces(Render::Interfaces* p)
    {
        p->pHAL             = pHAL;
        p->pRenderer2D      = pR2D;
        p->pTextureManager  = pHAL->GetTextureManager();
        p->RenderThreadID   = 0;
    }
    virtual void PushThreadCommand(ThreadCommand* command)
    {
        if (command) command->Execute();
    }
};

// ---------------------------------------------------------------------------
// Log: suppress output (headless)
// ---------------------------------------------------------------------------

class CaptureLog : public GFx::Log
{
public:
    virtual void LogMessageVarg(SF::LogMessageId, const char*, va_list) {}
};

// ---------------------------------------------------------------------------
// Main capture logic
// ---------------------------------------------------------------------------

static LRESULT CALLBACK NullWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    return DefWindowProcA(hwnd, msg, wp, lp);
}

int main(int argc, char* argv[])
{
    // --- Parse arguments ---
    if (argc < 3)
    {
        fprintf(stderr, "Usage: CaptureD3D11.exe <swf_path> <output_dir> --frames 1,10,30,60 [--width 1024] [--height 768]\n");
        return 1;
    }

    const char*       swfPath   = argv[1];
    const char*       outputDir = argv[2];
    std::vector<int>  frames;
    int               width  = 1024;
    int               height = 768;

    for (int i = 3; i < argc; i++)
    {
        if (_stricmp(argv[i], "--frames") == 0 && i + 1 < argc)
        {
            ++i;
            char buf[256];
            strncpy_s(buf, argv[i], 255);
            char* tok = strtok(buf, ",");
            while (tok)
            {
                frames.push_back(atoi(tok));
                tok = strtok(NULL, ",");
            }
        }
        else if (_stricmp(argv[i], "--width") == 0 && i + 1 < argc)
            width = atoi(argv[++i]);
        else if (_stricmp(argv[i], "--height") == 0 && i + 1 < argc)
            height = atoi(argv[++i]);
    }

    if (frames.empty())
    {
        frames.push_back(1);
        frames.push_back(10);
        frames.push_back(30);
        frames.push_back(60);
    }

    // Sort and find max frame
    std::sort(frames.begin(), frames.end());
    int maxFrame = frames.back();

    // Ensure output dir exists
    MakeDir(outputDir);

    // --- GFx system init ---
    SF::SysAllocMalloc sysAlloc;
    SF::GFx::System gfxInit(&sysAlloc);

    // --- Create hidden Win32 window (required for D3D device + swapchain) ---
    HINSTANCE hInst = GetModuleHandle(NULL);
    WNDCLASSEXA wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = NullWndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = "CaptureD3D11_Class";
    RegisterClassExA(&wc);

    HWND hWnd = CreateWindowExA(0, "CaptureD3D11_Class", "CaptureD3D11",
        WS_OVERLAPPEDWINDOW, 0, 0, width, height,
        NULL, NULL, hInst, NULL);
    if (!hWnd)
    {
        fprintf(stderr, "CaptureD3D11: Failed to create window\n");
        return 1;
    }
    // Keep hidden — we just need the handle for DXGI
    ShowWindow(hWnd, SW_HIDE);

    // --- Create D3D11 device + swapchain ---
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount                         = 1;
    scd.BufferDesc.Width                    = (UINT)width;
    scd.BufferDesc.Height                   = (UINT)height;
    scd.BufferDesc.Format                   = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator    = 60;
    scd.BufferDesc.RefreshRate.Denominator  = 1;
    scd.BufferUsage                         = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow                        = hWnd;
    scd.SampleDesc.Count                    = 1;
    scd.SampleDesc.Quality                  = 0;
    scd.Windowed                            = TRUE;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    ID3D11Device*        pDevice        = nullptr;
    ID3D11DeviceContext* pDeviceContext = nullptr;
    IDXGISwapChain*      pSwapChain     = nullptr;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
        0,
        featureLevels, 3, D3D11_SDK_VERSION,
        &scd, &pSwapChain, &pDevice, NULL, &pDeviceContext);

    if (FAILED(hr))
    {
        fprintf(stderr, "CaptureD3D11: D3D11CreateDeviceAndSwapChain failed (hr=0x%08X)\n", (unsigned)hr);
        return 1;
    }

    // --- Create render target view ---
    ID3D11Texture2D*        pBackBuffer    = nullptr;
    ID3D11RenderTargetView* pRenderTarget  = nullptr;
    ID3D11DepthStencilView* pDepthStencil  = nullptr;
    ID3D11Texture2D*        pDepthBuf      = nullptr;

    hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    if (FAILED(hr)) { fprintf(stderr, "CaptureD3D11: GetBuffer failed\n"); return 1; }

    hr = pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTarget);
    pBackBuffer->Release();
    pBackBuffer = nullptr;
    if (FAILED(hr)) { fprintf(stderr, "CaptureD3D11: CreateRenderTargetView failed\n"); return 1; }

    D3D11_TEXTURE2D_DESC dsd = {};
    dsd.Width               = (UINT)width;
    dsd.Height              = (UINT)height;
    dsd.MipLevels           = 1;
    dsd.ArraySize           = 1;
    dsd.Format              = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsd.SampleDesc.Count    = 1;
    dsd.SampleDesc.Quality  = 0;
    dsd.Usage               = D3D11_USAGE_DEFAULT;
    dsd.BindFlags           = D3D11_BIND_DEPTH_STENCIL;
    hr = pDevice->CreateTexture2D(&dsd, NULL, &pDepthBuf);
    if (FAILED(hr)) { fprintf(stderr, "CaptureD3D11: CreateTexture2D (depth) failed\n"); return 1; }

    hr = pDevice->CreateDepthStencilView(pDepthBuf, NULL, &pDepthStencil);
    if (FAILED(hr)) { fprintf(stderr, "CaptureD3D11: CreateDepthStencilView failed\n"); return 1; }

    pDeviceContext->OMSetRenderTargets(1, &pRenderTarget, pDepthStencil);

    D3D11_VIEWPORT vp = {};
    vp.Width    = (FLOAT)width;
    vp.Height   = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    pDeviceContext->RSSetViewports(1, &vp);

    // --- Create staging texture for readback ---
    ID3D11Texture2D* pStaging = nullptr;
    D3D11_TEXTURE2D_DESC stagDesc = {};
    stagDesc.Width          = (UINT)width;
    stagDesc.Height         = (UINT)height;
    stagDesc.MipLevels      = 1;
    stagDesc.ArraySize      = 1;
    stagDesc.Format         = DXGI_FORMAT_R8G8B8A8_UNORM;
    stagDesc.SampleDesc.Count = 1;
    stagDesc.Usage          = D3D11_USAGE_STAGING;
    stagDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    hr = pDevice->CreateTexture2D(&stagDesc, NULL, &pStaging);
    if (FAILED(hr)) { fprintf(stderr, "CaptureD3D11: CreateTexture2D (staging) failed\n"); return 1; }

    // --- Initialize Scaleform HAL ---
    CaptureThreadCommandQueue* queue = new CaptureThreadCommandQueue();
    Ptr<Render::D3D1x::HAL> pRenderHAL = *new Render::D3D1x::HAL(queue);
    Ptr<Render::Renderer2D> pRenderer;
    if (!(pRenderer = *new Render::Renderer2D(pRenderHAL.GetPtr())))
    {
        fprintf(stderr, "CaptureD3D11: Failed to create Renderer2D\n");
        return 1;
    }

    queue->pHAL = pRenderHAL;
    queue->pR2D = pRenderer;

    if (!pRenderHAL->InitHAL(D3D1x::HALInitParams(pDevice, pDeviceContext)))
    {
        fprintf(stderr, "CaptureD3D11: HAL init failed\n");
        return 1;
    }

    // --- Set up loader ---
    Loader loader;
    loader.SetLog(Ptr<GFx::Log>(*new CaptureLog()));

    Ptr<ActionControl> pActCtrl = *new ActionControl(ActionControl::Action_ErrorSuppress);
    loader.SetActionControl(pActCtrl);

    Ptr<FileOpener> pFileOpener = *new FileOpener;
    loader.SetFileOpener(pFileOpener);

    Ptr<FontProviderWin32> fontProvider = *new FontProviderWin32(::GetDC(0));
    loader.SetFontProvider(fontProvider);

    Ptr<GFx::ImageFileHandlerRegistry> pImgReg = *new GFx::ImageFileHandlerRegistry();
#ifdef SF_ENABLE_LIBJPEG
    pImgReg->AddHandler(&SF::Render::JPEG::FileReader::Instance);
#endif
#ifdef SF_ENABLE_LIBPNG
    pImgReg->AddHandler(&SF::Render::PNG::FileReader::Instance);
#endif
    pImgReg->AddHandler(&SF::Render::TGA::FileReader::Instance);
    pImgReg->AddHandler(&SF::Render::DDS::FileReader::Instance);
    loader.SetImageFileHandlerRegistry(pImgReg);

    Ptr<ASSupport> pAS3Support = *new GFx::AS3Support();
    loader.SetAS3Support(pAS3Support);
    Ptr<ASSupport> pAS2Support = *new GFx::AS2Support();
    loader.SetAS2Support(pAS2Support);

    // --- Load SWF ---
    Ptr<MovieDef> pMovieDef = *loader.CreateMovie(swfPath, Loader::LoadAll);
    if (!pMovieDef)
    {
        fprintf(stderr, "CaptureD3D11: Failed to load SWF: %s\n", swfPath);
        return 1;
    }

    Ptr<Movie> pMovie = *pMovieDef->CreateInstance(false, 0, 0, queue);
    if (!pMovie)
    {
        fprintf(stderr, "CaptureD3D11: Failed to create movie instance\n");
        return 1;
    }

    pMovie->SetViewport(width, height, 0, 0, width, height);
    pMovie->SetBackgroundAlpha(0.0f);
    pMovie->SetMouseCursorCount(1);

    // --- Render loop ---
    int       frameNumber = 0; // 1-based frame counter for user
    int       captureIdx  = 0;
    const float kFrameDt = 1.0f / 30.0f;

    // Precompute which internal loop iterations (0-based) correspond to capture frames.
    // Frame 1 = after first Advance, loop iteration 0.
    // Frame N = loop iteration N-1.
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Process Windows messages to keep the window alive (required for DXGI)
    MSG msg;

    while (captureIdx < (int)frames.size())
    {
        // Pump messages
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        frameNumber++;

        // Advance movie by one fixed step
        pMovie->Advance(kFrameDt, 0);

        // Clear and render
        pDeviceContext->ClearRenderTargetView(pRenderTarget, clearColor);
        pDeviceContext->ClearDepthStencilView(pDepthStencil,
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        pRenderer->BeginFrame();
        MovieDisplayHandle hmd = pMovie->GetDisplayHandle();
        if (hmd.NextCapture(pRenderer->GetContextNotify()))
            pRenderer->Display(hmd);
        pRenderer->EndFrame();

        pSwapChain->Present(0, 0);

        // Capture this frame if requested
        if (captureIdx < (int)frames.size() && frameNumber == frames[captureIdx])
        {
            // Readback: get current back buffer and copy to staging
            ID3D11Texture2D* pCurBack = nullptr;
            hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pCurBack);
            if (SUCCEEDED(hr) && pCurBack)
            {
                pDeviceContext->CopyResource(pStaging, pCurBack);
                pCurBack->Release();

                D3D11_MAPPED_SUBRESOURCE mapped = {};
                hr = pDeviceContext->Map(pStaging, 0, D3D11_MAP_READ, 0, &mapped);
                if (SUCCEEDED(hr))
                {
                    char outPath[MAX_PATH];
                    _snprintf_s(outPath, MAX_PATH, _TRUNCATE, "%s\\frame_%04d.ppm",
                        outputDir, frameNumber);

                    bool ok = WritePPM(outPath,
                        (const uint8_t*)mapped.pData,
                        (uint32_t)width, (uint32_t)height,
                        mapped.RowPitch);

                    pDeviceContext->Unmap(pStaging, 0);

                    if (ok)
                        printf("Captured frame %d -> %s\n", frameNumber, outPath);
                    else
                        fprintf(stderr, "CaptureD3D11: Failed to write PPM: %s\n", outPath);
                }
                else
                {
                    fprintf(stderr, "CaptureD3D11: Map staging failed at frame %d\n", frameNumber);
                }
            }
            captureIdx++;
        }

        if (frameNumber >= maxFrame)
            break;
    }

    // --- Cleanup ---
    pMovie.Clear();
    pMovieDef.Clear();
    delete queue;
    pRenderer.Clear();
    pRenderHAL->ShutdownHAL();
    pRenderHAL.Clear();

    if (pStaging)     pStaging->Release();
    if (pDepthStencil) pDepthStencil->Release();
    if (pDepthBuf)    pDepthBuf->Release();
    if (pRenderTarget) pRenderTarget->Release();
    if (pSwapChain)   pSwapChain->Release();
    if (pDeviceContext) pDeviceContext->Release();
    if (pDevice)      pDevice->Release();

    DestroyWindow(hWnd);

    printf("CaptureD3D11: Done.\n");
    return 0;
}
