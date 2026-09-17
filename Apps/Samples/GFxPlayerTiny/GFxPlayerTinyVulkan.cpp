/**************************************************************************

Filename    :   GFxPlayerTinyVulkan.cpp
Content     :   Tiny Scaleform player using the Vulkan rendering backend.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <windows.h>
#include <winuser.h>
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL   0x020A
#define WHEEL_DELTA     120
#endif
#include <stdio.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

#include "GFx_Kernel.h"
#include "GFx.h"
#include "GFx/GFx_AS3Support.h"
#include "GFx/AS3/AS3_Global.h"
#include "GFx/GFx_ImageCreator.h"
#include "GFx_Renderer_Vulkan.h"
#include "Render/Renderer2D.h"

#ifdef GFX_ENABLE_VIDEO
#include "Video/Video_Video.h"
#include "Video/Video_VideoPC.h"
#include "Video/Video_VideoSoundSystemDX8.h"
#endif

using namespace Scaleform;
using namespace Scaleform::Render;
using namespace Scaleform::GFx;

// Globals
static HWND                     hWnd = NULL;
static bool                     Running = true;
static bool                     NeedsResize = false;
static bool                     ControlKeyDown = false;
static unsigned                 WindowWidth  = 1024;
static unsigned                 WindowHeight = 768;

// Vulkan objects
static VkInstance               vkInstance   = VK_NULL_HANDLE;
static VkPhysicalDevice         vkPhysDevice = VK_NULL_HANDLE;
static VkDevice                 vkDevice     = VK_NULL_HANDLE;
static VkQueue                  vkQueue      = VK_NULL_HANDLE;
static uint32_t                 vkQueueFamily = 0;
static VkSurfaceKHR             vkSurface    = VK_NULL_HANDLE;
static VkSwapchainKHR           vkSwapchain  = VK_NULL_HANDLE;
static VkFormat                 vkSwapFormat = VK_FORMAT_B8G8R8A8_UNORM;
static VkExtent2D               vkSwapExtent = { 0, 0 };
static VkRenderPass             vkRenderPass       = VK_NULL_HANDLE;
static VkRenderPass             vkResumeRenderPass = VK_NULL_HANDLE; // LOAD_OP_LOAD, for post-filter restart
static VkCommandPool            vkCmdPool    = VK_NULL_HANDLE;

static const unsigned           MaxFramesInFlight = 2;
static VkCommandBuffer          vkCmdBuffers[MaxFramesInFlight] = {};
static VkSemaphore              vkImageAvailable[MaxFramesInFlight] = {};
static VkSemaphore              vkRenderFinished[MaxFramesInFlight] = {};
static VkFence                  vkInFlightFences[MaxFramesInFlight] = {};

static VkImage                  vkSwapImages[8] = {};
static VkImageView              vkSwapViews[8]  = {};
static VkFramebuffer            vkSwapFBs[8]    = {};
static uint32_t                 vkSwapCount     = 0;

static VkImage                  vkDepthImage  = VK_NULL_HANDLE;
static VkImageView              vkDepthView   = VK_NULL_HANDLE;
static VkDeviceMemory           vkDepthMemory = VK_NULL_HANDLE;
static VkFormat                 vkDepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

static uint32_t                 CurrentFrame  = 0;
static uint32_t                 CurrentImage  = 0;

static FILE* vkLogFile = nullptr;
static int   MaxFrames = -1;   // -1 = run until window close; set via --frames N

#if defined(SF_BUILD_DEBUG) || !defined(NDEBUG)
static VkDebugUtilsMessengerEXT vkDebugMessenger = VK_NULL_HANDLE;

static VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* pData,
    void* pUser)
{
    (void)pUser;
    // Capture WARNING+ for standard validation; also capture INFO from
    // BEST_PRACTICES and PERFORMANCE channels so no hints are missed.
    bool isBestPractices = (type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0
                        || (pData->messageIdNumber != 0 && strstr(pData->pMessageIdName ? pData->pMessageIdName : "", "BestPractices") != nullptr);
    bool shouldLog = (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                  || (isBestPractices && severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT);
    if (!shouldLog) return VK_FALSE;

    const char* level =
        (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)   ? "ERROR" :
        (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) ? "WARN"  : "INFO";
    const char* typeTag =
        (type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)  ? "BEST_PRACTICES" :
        (type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)   ? "VALIDATION"     : "GENERAL";

    fprintf(stderr, "[VK %s/%s] %s\n", level, typeTag, pData->pMessage);
    fflush(stderr);
    OutputDebugStringA(pData->pMessage);
    OutputDebugStringA("\n");
    if (vkLogFile)
    {
        fprintf(vkLogFile, "[VK %s/%s] %s\n", level, typeTag, pData->pMessage);
        fflush(vkLogFile);
    }
    return VK_FALSE;
}
#endif

// GFx log handler: forwards all messages (script errors, AS3 trace, action log) to vulkan_debug.log
class GFxFileLog : public Scaleform::Log
{
public:
    virtual void LogMessageVarg(Scaleform::LogMessageId messageType, const char* fmt, va_list argList)
    {
        if (!vkLogFile) return;
        const char* prefix;
        switch (messageType.GetChannel())
        {
        case Scaleform::LogChannel_Script: prefix = "[AS3 Script]"; break;
        case Scaleform::LogChannel_Action: prefix = "[AS3 Action]"; break;
        case Scaleform::LogChannel_Parse:  prefix = "[GFx Parse]";  break;
        default:                           prefix = "[GFx]";        break;
        }
        fprintf(vkLogFile, "%s ", prefix);
        vfprintf(vkLogFile, fmt, argList);
        size_t len = strlen(fmt);
        if (len == 0 || fmt[len - 1] != '\n') fprintf(vkLogFile, "\n");
        fflush(vkLogFile);
    }
};

// Single-threaded command queue: provides render interfaces (HAL, Renderer2D,
// TextureManager) to the movie system so that video textures can be created.
class VulkanThreadCommandQueue : public Render::ThreadCommandQueue
{
public:
    virtual void GetRenderInterfaces(Render::Interfaces* p)
    {
        p->pHAL = pHAL;
        p->pRenderer2D = pR2D;
        p->pTextureManager = pHAL ? pHAL->GetTextureManager() : nullptr;
        p->RenderThreadID = 0;
    }
    virtual void PushThreadCommand(ThreadCommand* command)
    {
        if (command) command->Execute();
    }
    Render::HAL*        pHAL;
    Render::Renderer2D* pR2D;
};

// Scaleform objects
static Ptr<Vulkan::HAL>        pHAL;
static Ptr<Renderer2D>         pRenderer;
static Loader*                 pLoader = nullptr;
static Ptr<MovieDef>           pMovieDef;
static Ptr<Movie>              pMovie;
static VulkanThreadCommandQueue* pCommandQueue = nullptr;
#ifdef GFX_ENABLE_VIDEO
static Ptr<GFx::Video::Video>  pVideo;
#endif

// Forward declarations
bool    InitVulkan();
void    CleanupVulkan();
bool    CreateSwapchain();
void    CleanupSwapchain();
bool    CreateDepthResources();
bool    CreateRenderPass();
bool    CreateFramebuffers();

// Capture the current swapchain image (after submit, before present) and write a PPM file to disk.
// The swapchain image must be in PRESENT_SRC_KHR layout when called.
// Stalls the GPU for one frame - use only for debugging.
static void CaptureScreenshot(const char* filename, uint32_t imageIdx)
{
    if (!vkDevice || !vkPhysDevice || !vkQueue || !vkCmdPool) return;

    vkDeviceWaitIdle(vkDevice);

    uint32_t w = vkSwapExtent.width;
    uint32_t h = vkSwapExtent.height;
    VkDeviceSize bufSize = (VkDeviceSize)w * h * 4;

    // Staging buffer (host-visible, host-coherent)
    VkBuffer      stagingBuf = VK_NULL_HANDLE;
    VkDeviceMemory stagingMem = VK_NULL_HANDLE;

    VkBufferCreateInfo bci = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bci.size        = bufSize;
    bci.usage       = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(vkDevice, &bci, nullptr, &stagingBuf) != VK_SUCCESS) return;

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(vkDevice, stagingBuf, &memReq);

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(vkPhysDevice, &memProps);
    uint32_t memTypeIdx = UINT32_MAX;
    VkMemoryPropertyFlags needed = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
    {
        if ((memReq.memoryTypeBits & (1u << i)) && (memProps.memoryTypes[i].propertyFlags & needed) == needed)
        { memTypeIdx = i; break; }
    }
    if (memTypeIdx == UINT32_MAX) { vkDestroyBuffer(vkDevice, stagingBuf, nullptr); return; }

    VkMemoryAllocateInfo mai = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    mai.allocationSize  = memReq.size;
    mai.memoryTypeIndex = memTypeIdx;
    if (vkAllocateMemory(vkDevice, &mai, nullptr, &stagingMem) != VK_SUCCESS)
    { vkDestroyBuffer(vkDevice, stagingBuf, nullptr); return; }
    vkBindBufferMemory(vkDevice, stagingBuf, stagingMem, 0);

    // One-shot command buffer
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo cbAI = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cbAI.commandPool        = vkCmdPool;
    cbAI.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbAI.commandBufferCount = 1;
    vkAllocateCommandBuffers(vkDevice, &cbAI, &cmd);

    VkCommandBufferBeginInfo cbBI = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    cbBI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &cbBI);

    // PRESENT_SRC_KHR -> TRANSFER_SRC_OPTIMAL
    VkImageMemoryBarrier bar = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    bar.srcAccessMask       = VK_ACCESS_MEMORY_READ_BIT;
    bar.dstAccessMask       = VK_ACCESS_TRANSFER_READ_BIT;
    bar.oldLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    bar.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    bar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bar.image               = vkSwapImages[imageIdx];
    bar.subresourceRange    = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &bar);

    VkBufferImageCopy region = {};
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageExtent      = { w, h, 1 };
    vkCmdCopyImageToBuffer(cmd, vkSwapImages[imageIdx],
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuf, 1, &region);

    // TRANSFER_SRC_OPTIMAL -> PRESENT_SRC_KHR
    bar.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    bar.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    bar.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    bar.newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0, 0, nullptr, 0, nullptr, 1, &bar);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo si = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    si.commandBufferCount = 1;
    si.pCommandBuffers    = &cmd;
    vkQueueSubmit(vkQueue, 1, &si, VK_NULL_HANDLE);
    vkQueueWaitIdle(vkQueue);
    vkFreeCommandBuffers(vkDevice, vkCmdPool, 1, &cmd);

    // Map staging buffer and write binary PPM (P6)
    void* mapped = nullptr;
    vkMapMemory(vkDevice, stagingMem, 0, bufSize, 0, &mapped);
    FILE* ppm = fopen(filename, "wb");
    if (ppm)
    {
        fprintf(ppm, "P6\n%u %u\n255\n", w, h);
        const uint8_t* px = (const uint8_t*)mapped;
        for (uint32_t i = 0; i < w * h; i++, px += 4)
        {
            // Swapchain is B8G8R8A8 — swap R/B for PPM RGB
            fputc(px[2], ppm);
            fputc(px[1], ppm);
            fputc(px[0], ppm);
        }
        fclose(ppm);
        if (vkLogFile)
        {
            fprintf(vkLogFile, "Screenshot saved: %s (%ux%u)\n", filename, w, h);
            fflush(vkLogFile);
        }
    }
    vkUnmapMemory(vkDevice, stagingMem);
    vkDestroyBuffer(vkDevice, stagingBuf, nullptr);
    vkFreeMemory(vkDevice, stagingMem, nullptr);
}


// -------------------------------------------------------------------------
// Input helpers – mirror the DX player's OnMouse / OnKey interface
// -------------------------------------------------------------------------

static void OnMouse(GFx::Event::EventType eventType, unsigned button, int x, int y)
{
    if (pMovie)
    {
        MouseEvent mevent(eventType, button, (float)x, (float)y);
        pMovie->HandleEvent(mevent);
    }
}

static void OnMouseWheel(int zdelta, int x, int y)
{
    if (pMovie)
    {
        MouseEvent mevent(GFx::Event::MouseWheel, 0, (float)x, (float)y,
                          (float)((zdelta / WHEEL_DELTA) * 3));
        pMovie->HandleEvent(mevent);
    }
}

static void HandleKeyEvent(unsigned keyCode, bool downFlag)
{
    Key::Code key(Key::None);

    if (keyCode >= 'A' && keyCode <= 'Z')
    {
        key = (Key::Code)((keyCode - 'A') + Key::A);
    }
    else if (keyCode >= VK_F1 && keyCode <= VK_F15)
    {
        key = (Key::Code)((keyCode - VK_F1) + Key::F1);
    }
    else if (keyCode >= '0' && keyCode <= '9')
    {
        key = (Key::Code)((keyCode - '0') + 48);
    }
    else if (keyCode >= VK_NUMPAD0 && keyCode <= VK_NUMPAD9)
    {
        key = (Key::Code)((keyCode - VK_NUMPAD0) + Key::KP_0);
    }
    else
    {
        static const struct { int vk; Key::Code gs; } table[] =
        {
            { VK_SHIFT,    Key::Shift    },
            { VK_RSHIFT,   Key::Shift    },
            { VK_CONTROL,  Key::Control  },
            { VK_RCONTROL, Key::Control  },
            { VK_MENU,     Key::Alt      },
            { VK_RMENU,    Key::Alt      },
            { VK_RETURN,   Key::Return   },
            { VK_ESCAPE,   Key::Escape   },
            { VK_LEFT,     Key::Left     },
            { VK_UP,       Key::Up       },
            { VK_RIGHT,    Key::Right    },
            { VK_DOWN,     Key::Down     },
            { VK_SPACE,    Key::Space    },
            { VK_BACK,     Key::Backspace},
            { VK_DELETE,   Key::Delete   },
            { VK_INSERT,   Key::Insert   },
            { VK_HOME,     Key::Home     },
            { VK_END,      Key::End      },
            { VK_PRIOR,    Key::PageUp   },
            { VK_NEXT,     Key::PageDown },
            { VK_TAB,      Key::Tab      },
            { 0, Key::None }
        };
        for (int i = 0; table[i].vk != 0; i++)
        {
            if (keyCode == (unsigned)table[i].vk)
            { key = table[i].gs; break; }
        }
    }

    if (key != Key::None && pMovie)
    {
        KeyEvent event(downFlag ? GFx::Event::KeyDown : GFx::Event::KeyUp, key);
        pMovie->HandleEvent(event);
    }
}

static void OnKey(unsigned keyCode, bool downFlag)
{
    if (!pMovie) return;

    if (keyCode == VK_CONTROL)
    {
        ControlKeyDown = downFlag;
        return;
    }
    if (keyCode == VK_MENU && downFlag)
        ControlKeyDown = false;

    if (!ControlKeyDown)
        HandleKeyEvent(keyCode, downFlag);
}

// -------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
    case WM_CLOSE:
        Running = false;
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            WindowWidth  = LOWORD(lParam);
            WindowHeight = HIWORD(lParam);
            NeedsResize  = true;
        }
        return 0;

    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT)
        {
            ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
            return 0;
        }
        break;

    case WM_MOUSEMOVE:
        OnMouse(GFx::Event::MouseMove, 0, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
        return 0;

    case WM_LBUTTONDOWN:
        ::SetCapture(hwnd);
        OnMouse(GFx::Event::MouseDown, 0, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
        return 0;
    case WM_LBUTTONUP:
        ::ReleaseCapture();
        OnMouse(GFx::Event::MouseUp, 0, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
        return 0;

    case WM_RBUTTONDOWN:
        OnMouse(GFx::Event::MouseDown, 1, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
        return 0;
    case WM_RBUTTONUP:
        OnMouse(GFx::Event::MouseUp, 1, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
        return 0;

    case WM_MBUTTONDOWN:
        OnMouse(GFx::Event::MouseDown, 2, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
        return 0;
    case WM_MBUTTONUP:
        OnMouse(GFx::Event::MouseUp, 2, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
        return 0;

    case WM_MOUSEWHEEL:
        {
            // lParam carries screen-space position; convert to client coords
            POINT wcl = { 0, 0 };
            ::ClientToScreen(hwnd, &wcl);
            int mx = (int)(short)LOWORD(lParam) - wcl.x;
            int my = (int)(short)HIWORD(lParam) - wcl.y;
            OnMouseWheel((int)(short)HIWORD(wParam), mx, my);
            return 0;
        }

    case WM_KEYDOWN:
        OnKey((unsigned)wParam, true);
        return 0;
    case WM_KEYUP:
        OnKey((unsigned)wParam, false);
        return 0;

    case WM_CHAR:
        {
            UInt32 wcharCode = (UInt32)wParam;
            if (pMovie && wcharCode)
            {
                CharEvent event(wcharCode);
                pMovie->HandleEvent(event);
            }
        }
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}


bool InitWindow()
{
    WNDCLASSEXA wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "ScaleformVulkanDemo";
    RegisterClassExA(&wc);

    RECT rc = { 0, 0, (LONG)WindowWidth, (LONG)WindowHeight };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindowExA(0, "ScaleformVulkanDemo", "Scaleform Vulkan Demo",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    return hWnd != NULL;
}


bool InitVulkan()
{
    // Instance
    VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = "ScaleformVulkanDemo";
    appInfo.apiVersion       = VK_API_VERSION_1_0;

    const char* exts[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#if defined(SF_BUILD_DEBUG) || !defined(NDEBUG)
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
#endif
    };
    VkInstanceCreateInfo instCI = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instCI.pApplicationInfo        = &appInfo;
    instCI.ppEnabledExtensionNames = exts;
#if defined(SF_BUILD_DEBUG) || !defined(NDEBUG)
    instCI.enabledExtensionCount   = 4;

    const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
    bool validationAvailable = false;
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        if (layerCount > 0)
        {
            VkLayerProperties* availLayers = (VkLayerProperties*)_alloca(sizeof(VkLayerProperties) * layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availLayers);
            for (uint32_t i = 0; i < layerCount; i++)
            {
                if (strcmp(availLayers[i].layerName, "VK_LAYER_KHRONOS_validation") == 0)
                { validationAvailable = true; break; }
            }
        }
    }
    VkValidationFeatureEnableEXT bestPracticesFeatures[] = {
        VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
    };
    VkValidationFeaturesEXT validationFeatures = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
    validationFeatures.enabledValidationFeatureCount = 1;
    validationFeatures.pEnabledValidationFeatures    = bestPracticesFeatures;

    if (validationAvailable)
    {
        instCI.enabledLayerCount   = 1;
        instCI.ppEnabledLayerNames = layers;
        instCI.pNext               = &validationFeatures;
        if (vkLogFile) { fprintf(vkLogFile, "Vulkan validation layers: ENABLED (+ BEST_PRACTICES)\n"); fflush(vkLogFile); }
    }
    else
    {
        if (vkLogFile) { fprintf(vkLogFile, "Vulkan validation layers: NOT AVAILABLE\n"); fflush(vkLogFile); }
    }
#else
    instCI.enabledExtensionCount   = 2;
#endif
    if (vkCreateInstance(&instCI, nullptr, &vkInstance) != VK_SUCCESS)
        return false;

#if defined(SF_BUILD_DEBUG) || !defined(NDEBUG)
    auto pfnCreate = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");
    if (pfnCreate)
    {
        VkDebugUtilsMessengerCreateInfoEXT dbgCI = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        dbgCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dbgCI.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        dbgCI.pfnUserCallback = vkDebugCallback;
        pfnCreate(vkInstance, &dbgCI, nullptr, &vkDebugMessenger);
    }
#endif

    // Physical device
    uint32_t devCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &devCount, nullptr);
    if (!devCount) return false;
    VkPhysicalDevice devs[16];
    devCount = (devCount > 16) ? 16 : devCount;
    vkEnumeratePhysicalDevices(vkInstance, &devCount, devs);
    vkPhysDevice = devs[0];
    for (uint32_t i = 0; i < devCount; i++)
    {
        VkPhysicalDeviceProperties p;
        vkGetPhysicalDeviceProperties(devs[i], &p);
        if (p.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        { vkPhysDevice = devs[i]; break; }
    }

    // Queue family
    uint32_t qfCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysDevice, &qfCount, nullptr);
    VkQueueFamilyProperties qfProps[32];
    qfCount = (qfCount > 32) ? 32 : qfCount;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysDevice, &qfCount, qfProps);
    for (uint32_t i = 0; i < qfCount; i++)
    {
        if (qfProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        { vkQueueFamily = i; break; }
    }

    // Logical device
    float prio = 1.0f;
    VkDeviceQueueCreateInfo qCI = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    qCI.queueFamilyIndex = vkQueueFamily;
    qCI.queueCount = 1;
    qCI.pQueuePriorities = &prio;

    const char* devExts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkPhysicalDeviceFeatures features = {};
    features.fillModeNonSolid = VK_TRUE;

    VkDeviceCreateInfo dCI = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    dCI.queueCreateInfoCount    = 1;
    dCI.pQueueCreateInfos       = &qCI;
    dCI.enabledExtensionCount   = 1;
    dCI.ppEnabledExtensionNames = devExts;
    dCI.pEnabledFeatures        = &features;
    if (vkCreateDevice(vkPhysDevice, &dCI, nullptr, &vkDevice) != VK_SUCCESS)
        return false;
    vkGetDeviceQueue(vkDevice, vkQueueFamily, 0, &vkQueue);

    // Surface
    VkWin32SurfaceCreateInfoKHR surfCI = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    surfCI.hinstance = GetModuleHandle(NULL);
    surfCI.hwnd      = hWnd;
    if (vkCreateWin32SurfaceKHR(vkInstance, &surfCI, nullptr, &vkSurface) != VK_SUCCESS)
        return false;

    // Verify present support on chosen queue family
    VkBool32 presentSupport = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysDevice, vkQueueFamily, vkSurface, &presentSupport);
    if (!presentSupport)
    {
        OutputDebugStringA("Vulkan: Chosen queue family does not support present\n");
        return false;
    }

    // Pick a supported surface format
    {
        uint32_t fmtCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysDevice, vkSurface, &fmtCount, nullptr);
        if (fmtCount > 0)
        {
            VkSurfaceFormatKHR fmts[64];
            fmtCount = (fmtCount > 64) ? 64 : fmtCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysDevice, vkSurface, &fmtCount, fmts);
            vkSwapFormat = fmts[0].format;
            for (uint32_t i = 0; i < fmtCount; i++)
            {
                if (fmts[i].format == VK_FORMAT_B8G8R8A8_UNORM)
                { vkSwapFormat = fmts[i].format; break; }
            }
        }
    }

    // Command pool
    VkCommandPoolCreateInfo cpCI = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    cpCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cpCI.queueFamilyIndex = vkQueueFamily;
    if (vkCreateCommandPool(vkDevice, &cpCI, nullptr, &vkCmdPool) != VK_SUCCESS)
        return false;

    // Command buffers
    VkCommandBufferAllocateInfo cbAI = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cbAI.commandPool        = vkCmdPool;
    cbAI.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbAI.commandBufferCount = MaxFramesInFlight;
    vkAllocateCommandBuffers(vkDevice, &cbAI, vkCmdBuffers);

    // Sync objects
    VkSemaphoreCreateInfo semCI = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenCI = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (unsigned i = 0; i < MaxFramesInFlight; i++)
    {
        vkCreateSemaphore(vkDevice, &semCI, nullptr, &vkImageAvailable[i]);
        vkCreateSemaphore(vkDevice, &semCI, nullptr, &vkRenderFinished[i]);
        vkCreateFence(vkDevice, &fenCI, nullptr, &vkInFlightFences[i]);
    }

    if (!CreateSwapchain()) return false;
    if (!CreateDepthResources()) return false;
    if (!CreateRenderPass()) return false;
    if (!CreateFramebuffers()) return false;

    return true;
}


bool CreateSwapchain()
{
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysDevice, vkSurface, &caps);
    vkSwapExtent.width  = Alg::Clamp(WindowWidth,  caps.minImageExtent.width,  caps.maxImageExtent.width);
    vkSwapExtent.height = Alg::Clamp(WindowHeight, caps.minImageExtent.height, caps.maxImageExtent.height);

    // Surface caps can report (0,0) when the window is minimised or not yet shown.
    // Creating a swapchain with zero extent is a spec violation
    // (VUID-VkSwapchainCreateInfoKHR-imageExtent-01689).
    if (vkSwapExtent.width == 0 || vkSwapExtent.height == 0)
    {
        if (vkLogFile) { fprintf(vkLogFile, "CreateSwapchain: extent is 0x0, deferring\n"); fflush(vkLogFile); }
        return false;
    }

    uint32_t imgCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imgCount > caps.maxImageCount) imgCount = caps.maxImageCount;

    VkSwapchainCreateInfoKHR ci = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    ci.surface          = vkSurface;
    ci.minImageCount    = imgCount;
    ci.imageFormat      = vkSwapFormat;
    ci.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    ci.imageExtent      = vkSwapExtent;
    ci.imageArrayLayers = 1;
    ci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.preTransform     = caps.currentTransform;
    ci.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
    ci.clipped          = VK_TRUE;
    if (vkCreateSwapchainKHR(vkDevice, &ci, nullptr, &vkSwapchain) != VK_SUCCESS)
        return false;

    vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &vkSwapCount, nullptr);
    if (vkSwapCount > 8) vkSwapCount = 8;
    vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &vkSwapCount, vkSwapImages);

    for (uint32_t i = 0; i < vkSwapCount; i++)
    {
        VkImageViewCreateInfo vCI = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        vCI.image = vkSwapImages[i]; vCI.viewType = VK_IMAGE_VIEW_TYPE_2D; vCI.format = vkSwapFormat;
        vCI.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        vkCreateImageView(vkDevice, &vCI, nullptr, &vkSwapViews[i]);
    }
    return true;
}

bool CreateDepthResources()
{
    VkFormat candidates[] = { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT };
    vkDepthFormat = VK_FORMAT_UNDEFINED;
    for (auto f : candidates)
    {
        VkFormatProperties p; vkGetPhysicalDeviceFormatProperties(vkPhysDevice, f, &p);
        if (p.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        { vkDepthFormat = f; break; }
    }
    if (vkDepthFormat == VK_FORMAT_UNDEFINED) return false;

    VkImageCreateInfo iCI = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    iCI.imageType = VK_IMAGE_TYPE_2D; iCI.format = vkDepthFormat;
    iCI.extent = { vkSwapExtent.width, vkSwapExtent.height, 1 };
    iCI.mipLevels = 1; iCI.arrayLayers = 1; iCI.samples = VK_SAMPLE_COUNT_1_BIT;
    iCI.tiling = VK_IMAGE_TILING_OPTIMAL; iCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkCreateImage(vkDevice, &iCI, nullptr, &vkDepthImage);

    VkMemoryRequirements mr; vkGetImageMemoryRequirements(vkDevice, vkDepthImage, &mr);
    VkMemoryAllocateInfo aI = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    aI.allocationSize = mr.size;
    aI.memoryTypeIndex = Vulkan::FindMemoryType(vkPhysDevice, mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkAllocateMemory(vkDevice, &aI, nullptr, &vkDepthMemory);
    vkBindImageMemory(vkDevice, vkDepthImage, vkDepthMemory, 0);

    VkImageViewCreateInfo vCI = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    vCI.image = vkDepthImage; vCI.viewType = VK_IMAGE_VIEW_TYPE_2D; vCI.format = vkDepthFormat;
    vCI.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
    return vkCreateImageView(vkDevice, &vCI, nullptr, &vkDepthView) == VK_SUCCESS;
}

bool CreateRenderPass()
{
    VkAttachmentDescription atts[2] = {};
    atts[0].format = vkSwapFormat; atts[0].samples = VK_SAMPLE_COUNT_1_BIT;
    atts[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; atts[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    atts[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; atts[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    atts[1].format = vkDepthFormat; atts[1].samples = VK_SAMPLE_COUNT_1_BIT;
    atts[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; atts[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; atts[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; atts[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference cRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference dRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
    VkSubpassDescription sp = {}; sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sp.colorAttachmentCount = 1; sp.pColorAttachments = &cRef; sp.pDepthStencilAttachment = &dRef;

    VkSubpassDependency dep = {};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL; dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dep.dstStageMask = dep.srcStageMask;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rpCI = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    rpCI.attachmentCount = 2; rpCI.pAttachments = atts;
    rpCI.subpassCount = 1; rpCI.pSubpasses = &sp;
    rpCI.dependencyCount = 1; rpCI.pDependencies = &dep;
    if (vkCreateRenderPass(vkDevice, &rpCI, nullptr, &vkRenderPass) != VK_SUCCESS) return false;

    // Resume render pass: identical but uses LOAD_OP_LOAD on both attachments.
    // Used by PopRenderTarget to restart the main pass after returning from an
    // offscreen filter render target, without discarding already-drawn content.
    atts[0].loadOp        = VK_ATTACHMENT_LOAD_OP_LOAD;
    atts[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    atts[1].loadOp        = VK_ATTACHMENT_LOAD_OP_LOAD;
    atts[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    atts[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    return vkCreateRenderPass(vkDevice, &rpCI, nullptr, &vkResumeRenderPass) == VK_SUCCESS;
}

bool CreateFramebuffers()
{
    for (uint32_t i = 0; i < vkSwapCount; i++)
    {
        VkImageView att[] = { vkSwapViews[i], vkDepthView };
        VkFramebufferCreateInfo fbCI = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        fbCI.renderPass = vkRenderPass; fbCI.attachmentCount = 2; fbCI.pAttachments = att;
        fbCI.width = vkSwapExtent.width; fbCI.height = vkSwapExtent.height; fbCI.layers = 1;
        if (vkCreateFramebuffer(vkDevice, &fbCI, nullptr, &vkSwapFBs[i]) != VK_SUCCESS)
            return false;
    }
    return true;
}

void CleanupSwapchain()
{
    if (vkDepthView)   { vkDestroyImageView(vkDevice, vkDepthView, nullptr); vkDepthView = VK_NULL_HANDLE; }
    if (vkDepthImage)  { vkDestroyImage(vkDevice, vkDepthImage, nullptr); vkDepthImage = VK_NULL_HANDLE; }
    if (vkDepthMemory) { vkFreeMemory(vkDevice, vkDepthMemory, nullptr); vkDepthMemory = VK_NULL_HANDLE; }
    for (uint32_t i = 0; i < vkSwapCount; i++)
    {
        if (vkSwapFBs[i])   { vkDestroyFramebuffer(vkDevice, vkSwapFBs[i], nullptr); vkSwapFBs[i] = VK_NULL_HANDLE; }
        if (vkSwapViews[i]) { vkDestroyImageView(vkDevice, vkSwapViews[i], nullptr); vkSwapViews[i] = VK_NULL_HANDLE; }
    }
    if (vkSwapchain) { vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr); vkSwapchain = VK_NULL_HANDLE; }
}

void CleanupVulkan()
{
    if (vkDevice) vkDeviceWaitIdle(vkDevice);
    for (unsigned i = 0; i < MaxFramesInFlight; i++)
    {
        if (vkImageAvailable[i]) vkDestroySemaphore(vkDevice, vkImageAvailable[i], nullptr);
        if (vkRenderFinished[i]) vkDestroySemaphore(vkDevice, vkRenderFinished[i], nullptr);
        if (vkInFlightFences[i]) vkDestroyFence(vkDevice, vkInFlightFences[i], nullptr);
    }
    if (vkCmdPool)   vkDestroyCommandPool(vkDevice, vkCmdPool, nullptr);
    if (vkResumeRenderPass) { vkDestroyRenderPass(vkDevice, vkResumeRenderPass, nullptr); vkResumeRenderPass = VK_NULL_HANDLE; }
    if (vkRenderPass) vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
    CleanupSwapchain();
    if (vkSurface)   vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
#if defined(SF_BUILD_DEBUG) || !defined(NDEBUG)
    if (vkDebugMessenger) {
        auto fn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT");
        if (fn) fn(vkInstance, vkDebugMessenger, nullptr);
    }
#endif
    if (vkDevice)    vkDestroyDevice(vkDevice, nullptr);
    if (vkInstance)  vkDestroyInstance(vkInstance, nullptr);
}


static int AppMain(LPSTR lpCmdLine);

static void WriteStackTrace(FILE* f, CONTEXT* ctx)
{
    HANDLE process = GetCurrentProcess();
    HANDLE thread  = GetCurrentThread();
    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
    SymInitialize(process, NULL, TRUE);

    STACKFRAME64 frame = {};
    frame.AddrPC.Mode    = AddrModeFlat;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;
#ifdef _M_X64
    DWORD machineType       = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Offset     = ctx->Rip;
    frame.AddrFrame.Offset  = ctx->Rbp;
    frame.AddrStack.Offset  = ctx->Rsp;
#else
    DWORD machineType       = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset     = ctx->Eip;
    frame.AddrFrame.Offset  = ctx->Ebp;
    frame.AddrStack.Offset  = ctx->Esp;
#endif

    char symbolBuf[sizeof(SYMBOL_INFO) + 256];
    SYMBOL_INFO* sym = (SYMBOL_INFO*)symbolBuf;
    sym->SizeOfStruct = sizeof(SYMBOL_INFO);
    sym->MaxNameLen   = 255;

    for (int i = 0; i < 64; i++)
    {
        if (!StackWalk64(machineType, process, thread, &frame, ctx,
                         NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
            break;
        if (frame.AddrPC.Offset == 0)
            break;

        DWORD64 displacement64 = 0;
        DWORD   displacement32 = 0;
        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        if (SymFromAddr(process, frame.AddrPC.Offset, &displacement64, sym))
        {
            if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &displacement32, &line))
                fprintf(f, "  [%02d] %s +0x%llx  (%s:%lu)\n", i, sym->Name, displacement64, line.FileName, line.LineNumber);
            else
                fprintf(f, "  [%02d] %s +0x%llx\n", i, sym->Name, displacement64);
        }
        else
        {
            fprintf(f, "  [%02d] 0x%016llx (no symbol)\n", i, frame.AddrPC.Offset);
        }
    }
    SymCleanup(process);
}

static LONG WINAPI CrashFilter(EXCEPTION_POINTERS* ep)
{
    FILE* f = vkLogFile;
    if (!f) f = fopen("vulkan_crash.log", "w");
    if (f)
    {
        fprintf(f, "=== CRASH ===\nException 0x%08lX at address 0x%p\n\nStack trace:\n",
                ep->ExceptionRecord->ExceptionCode, ep->ExceptionRecord->ExceptionAddress);
        fflush(f);
        WriteStackTrace(f, ep->ContextRecord);
        fprintf(f, "=== END ===\n");
        fflush(f);
        fclose(f);
    }
    vkLogFile = nullptr;
    return EXCEPTION_EXECUTE_HANDLER;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int)
{
#if defined(SF_BUILD_DEBUG) || !defined(NDEBUG)
    AllocConsole();
    FILE* conOut = nullptr;
    freopen_s(&conOut, "CONOUT$", "w", stdout);
    freopen_s(&conOut, "CONOUT$", "w", stderr);
    printf("=== Scaleform Vulkan Demo (Debug Console) ===\n");
    vkLogFile = fopen("vulkan_debug.log", "w");
#endif
    SetUnhandledExceptionFilter(CrashFilter);
    return AppMain(lpCmdLine);
}

static int AppMain(LPSTR lpCmdLine)
{
    GFx::System gfxInit;

    // Parse SWF path from command line, stripping quotes that Windows adds
    // when files are dragged onto the exe or passed via Explorer shell.
    static char swfBuf[MAX_PATH] = {};
    const char* swfPath = nullptr;
    if (lpCmdLine && lpCmdLine[0])
    {
        const char* src = lpCmdLine;
        while (*src == ' ' || *src == '"') src++;
        char* dst = swfBuf;
        while (*src && *src != '"' && (dst - swfBuf) < MAX_PATH - 1)
        {
            if (*src == ' ' && *(src + 1) == '-' && *(src + 2) == '-')
                break;
            *dst++ = *src++;
        }
        while (dst > swfBuf && *(dst-1) == ' ') dst--;
        *dst = '\0';
        if (swfBuf[0])
            swfPath = swfBuf;
    }
    if (!swfPath)
    {
        char exeDir[MAX_PATH];
        GetModuleFileNameA(NULL, exeDir, MAX_PATH);
        char* lastSlash = strrchr(exeDir, '\\');
        if (lastSlash) *(lastSlash + 1) = '\0';
        _snprintf_s(swfBuf, MAX_PATH, _TRUNCATE, "%s..\\..\\..\\..\\Data\\AS2\\Samples\\Window.swf", exeDir);
        swfPath = swfBuf;
    }

    // Parse --frames N: auto-exit after N rendered frames (for automated batch testing)
    if (lpCmdLine)
    {
        const char* framesOpt = strstr(lpCmdLine, "--frames ");
        if (framesOpt) MaxFrames = atoi(framesOpt + 9);
    }

    if (!InitWindow())
    {
        MessageBoxA(NULL, "Failed to create window", "Error", MB_OK);
        return 1;
    }

    if (!InitVulkan())
    {
        MessageBoxA(NULL, "Failed to initialize Vulkan", "Error", MB_OK);
        CleanupVulkan();
        return 1;
    }

    if (vkLogFile) { fprintf(vkLogFile, "Vulkan init succeeded. Initializing Scaleform HAL...\n"); fflush(vkLogFile); }

    // Initialize Scaleform
    pHAL = *SF_NEW Vulkan::HAL();
    if (vkLogFile) { fprintf(vkLogFile, "HAL object created.\n"); fflush(vkLogFile); }

    Vulkan::HALInitParams halParams(
        vkDevice, vkPhysDevice, vkQueue, vkQueueFamily,
        vkCmdPool, vkCmdBuffers[0], vkRenderPass, vkSwapFormat, vkInstance);

    if (!pHAL->InitHAL(halParams))
    {
        if (vkLogFile) { fprintf(vkLogFile, "HAL init FAILED\n"); fflush(vkLogFile); }
        pHAL.Clear();
        CleanupVulkan();
        return 1;
    }

    if (vkLogFile) { fprintf(vkLogFile, "HAL init succeeded. Creating renderer...\n"); fflush(vkLogFile); }

    Renderer2D* pRendererRaw = SF_NEW Renderer2D(pHAL.GetPtr());
    if (vkLogFile) { fprintf(vkLogFile, "Renderer2D constructed at %p\n", pRendererRaw); fflush(vkLogFile); }
    pRenderer = *pRendererRaw;
    if (vkLogFile) { fprintf(vkLogFile, "Renderer2D assigned.\n"); fflush(vkLogFile); }

    // Load SWF
    if (vkLogFile) { fprintf(vkLogFile, "Creating Loader...\n"); fflush(vkLogFile); }
    pLoader = new Loader();
    Ptr<ASSupport> pAS2Support = *new GFx::AS2Support();
    pLoader->SetAS2Support(pAS2Support);
    Ptr<ASSupport> pAS3Support = *new GFx::AS3Support();
    pLoader->SetAS3Support(pAS3Support);

    // Set up the single-threaded command queue so the movie system can access
    // the TextureManager (required for video texture creation and BitmapData).
    pCommandQueue = new VulkanThreadCommandQueue();
    pCommandQueue->pHAL = pHAL.GetPtr();
    pCommandQueue->pR2D = pRenderer.GetPtr();

    // Provide the GPU TextureManager so embedded SWF bitmaps are uploaded as GPU textures.
    Ptr<GFx::ImageCreator> pImageCreator = *SF_NEW GFx::ImageCreator(pHAL->GetTextureManager());
    pLoader->SetImageCreator(pImageCreator);

    // FileOpener is required so GFx can open SWF-referenced files (video .usm, images, etc.)
    Ptr<FileOpener> pFileOpener = *SF_NEW FileOpener;
    pLoader->SetFileOpener(pFileOpener);

    // Route all GFx/AS3 messages to the log file
    Ptr<GFxFileLog> pGFxLog = *SF_NEW GFxFileLog();
    pLoader->SetLog(pGFxLog);
    // Enable verbose ActionScript execution log (requires GFX_AS3_VERBOSE compile define)
    Ptr<GFx::ActionControl> pActionCtrl = *SF_NEW GFx::ActionControl(
        GFx::ActionControl::Action_Verbose | GFx::ActionControl::Action_LogAllFilenames);
    pLoader->SetActionControl(pActionCtrl);

#ifdef GFX_ENABLE_VIDEO
    // Video playback (CRI Sofdec2)
    pVideo = *new GFx::Video::VideoPC(GFx::Video::VideoVMSupportAll(),
        Thread::NormalPriority, 3, NULL);
    pVideo->SetSoundSystem(
        Ptr<GFx::Video::VideoSoundSystem>(*new GFx::Video::VideoSoundSystemDX8(0)));
    pVideo->SetTextureManager(pHAL->GetTextureManager());
    pLoader->SetVideo(pVideo);
    if (vkLogFile) { fprintf(vkLogFile, "Video support initialized (CRI Movie + DirectSound)\n"); fflush(vkLogFile); }
#endif

    if (vkLogFile) { fprintf(vkLogFile, "Loader created (AS2+AS3 support, verbose logging ON), loading SWF: %s\n", swfPath); fflush(vkLogFile); }
    pMovieDef = *pLoader->CreateMovie(swfPath, Loader::LoadAll);
    if (!pMovieDef)
    {
        if (vkLogFile) { fprintf(vkLogFile, "FAILED to load SWF: %s\n", swfPath); fflush(vkLogFile); }
    }
    else
    {
        if (vkLogFile) { fprintf(vkLogFile, "MovieDef loaded OK: %s\n", swfPath); fflush(vkLogFile); }
        pMovie = *pMovieDef->CreateInstance(false, 0, NULL, pCommandQueue);
        if (pMovie)
        {
            if (vkLogFile) { fprintf(vkLogFile, "Movie instance created, setting viewport %ux%u\n", WindowWidth, WindowHeight); fflush(vkLogFile); }
            pMovie->SetViewport(WindowWidth, WindowHeight, 0, 0, WindowWidth, WindowHeight);
            pMovie->SetBackgroundAlpha(0.0f);
            pMovie->SetMouseCursorCount(1);
        }
        else
        {
            if (vkLogFile) { fprintf(vkLogFile, "FAILED to create movie instance\n"); fflush(vkLogFile); }
        }
    }

    if (vkLogFile)
    {
        fprintf(vkLogFile, "========================================\n");
        fprintf(vkLogFile, "SWF: %s\n", swfPath);
        if (MaxFrames > 0)
            fprintf(vkLogFile, "Auto-exit after %d frames\n", MaxFrames);
        fprintf(vkLogFile, "Entering main loop...\n");
        fprintf(vkLogFile, "========================================\n");
        fflush(vkLogFile);
    }
    // Main loop
    MSG msg;
    unsigned loopCount = 0;
    while (Running)
    {
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) Running = false;
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (!Running) break;

        if (MaxFrames > 0 && (int)loopCount >= MaxFrames)
        {
            if (vkLogFile) { fprintf(vkLogFile, "Auto-exit: reached %d frames\n", MaxFrames); fflush(vkLogFile); }
            Running = false;
            break;
        }

        if (NeedsResize && WindowWidth > 0 && WindowHeight > 0)
        {
            vkDeviceWaitIdle(vkDevice);
            if (pHAL) pHAL->PrepareForReset();
            CleanupSwapchain();
            if (!CreateSwapchain() || !CreateDepthResources() || !CreateFramebuffers())
            {
                OutputDebugStringA("Vulkan: Swapchain resize failed, retrying next frame\n");
                if (pHAL) pHAL->RestoreAfterReset();
                NeedsResize = true;
                continue;
            }
            if (pHAL) pHAL->RestoreAfterReset();
            if (pMovie) pMovie->SetViewport(WindowWidth, WindowHeight, 0, 0, WindowWidth, WindowHeight);
            NeedsResize = false;
        }

        unsigned frameIdx = CurrentFrame % MaxFramesInFlight;
        vkWaitForFences(vkDevice, 1, &vkInFlightFences[frameIdx], VK_TRUE, UINT64_MAX);

        VkResult acquireResult = vkAcquireNextImageKHR(vkDevice, vkSwapchain, UINT64_MAX,
            vkImageAvailable[frameIdx], VK_NULL_HANDLE, &CurrentImage);
        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR || acquireResult == VK_SUBOPTIMAL_KHR)
        {
            NeedsResize = true;
            if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
                continue;
        }
        vkResetFences(vkDevice, 1, &vkInFlightFences[frameIdx]);

        VkCommandBuffer cmd = vkCmdBuffers[frameIdx];
        vkResetCommandBuffer(cmd, 0);
        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        vkBeginCommandBuffer(cmd, &beginInfo);

        VkClearValue clears[2] = {};
        clears[0].color = { { 0.2f, 0.2f, 0.3f, 1.0f } };
        clears[1].depthStencil = { 0.0f, 0 };

        VkRenderPassBeginInfo rpBI = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        rpBI.renderPass = vkRenderPass;
        rpBI.framebuffer = vkSwapFBs[CurrentImage];
        rpBI.renderArea.extent = vkSwapExtent;
        rpBI.clearValueCount = 2;
        rpBI.pClearValues = clears;
        vkCmdBeginRenderPass(cmd, &rpBI, VK_SUBPASS_CONTENTS_INLINE);

        // Render Scaleform
        if (pHAL && pMovie)
        {
            pHAL->SetCommandBuffer(cmd);
            pHAL->InRenderPass = true;
            // Provide the current swapchain framebuffer so PopRenderTarget can
            // resume the main render pass after an offscreen filter pass ends.
            pHAL->SetMainRenderTarget(vkRenderPass, vkResumeRenderPass,
                                      vkSwapFBs[CurrentImage], vkSwapExtent);
            Vulkan::RenderEvent::UpdateCommandBuffer(cmd);

            pMovie->Advance(1.0f / 30.0f, 0);

            pRenderer->BeginFrame();

            // Grab the RenderSync fence now (before EndFrame clears it)
            VkFence rsyncFence = VK_NULL_HANDLE;

            MovieDisplayHandle hmd = pMovie->GetDisplayHandle();
            if (hmd.NextCapture(pRenderer->GetContextNotify()))
            {
                pRenderer->Display(hmd);
                if (vkLogFile && loopCount < 3) { fprintf(vkLogFile, "  Display done (frame %u)\n", loopCount); fflush(vkLogFile); }
            }

            rsyncFence = pHAL->GetMeshCache().GetRenderSync()->GetNextEndFrameFence();
            pRenderer->EndFrame();

            if (!pHAL->InRenderPass)
            {
                vkCmdBeginRenderPass(cmd, &rpBI, VK_SUBPASS_CONTENTS_INLINE);
                pHAL->InRenderPass = true;
            }

            vkCmdEndRenderPass(cmd);
            vkEndCommandBuffer(cmd);

            VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores    = &vkImageAvailable[frameIdx];
            submitInfo.pWaitDstStageMask  = &waitStage;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers    = &cmd;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores  = &vkRenderFinished[frameIdx];
            vkQueueSubmit(vkQueue, 1, &submitInfo, vkInFlightFences[frameIdx]);

            if (rsyncFence != VK_NULL_HANDLE)
            {
                VkSubmitInfo emptySubmit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
                vkQueueSubmit(vkQueue, 1, &emptySubmit, rsyncFence);
            }
        }
        else
        {
            vkCmdEndRenderPass(cmd);
            vkEndCommandBuffer(cmd);

            VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores    = &vkImageAvailable[frameIdx];
            submitInfo.pWaitDstStageMask  = &waitStage;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers    = &cmd;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores  = &vkRenderFinished[frameIdx];
            vkQueueSubmit(vkQueue, 1, &submitInfo, vkInFlightFences[frameIdx]);
        }

        // Capture frames 10 and 60 to disk for automated analysis (after all submits, before present)
        if (loopCount == 10)
            CaptureScreenshot("screenshot_f10.ppm", CurrentImage);
        if (loopCount == 60)
            CaptureScreenshot("screenshot_f60.ppm", CurrentImage);

        VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &vkRenderFinished[frameIdx];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &vkSwapchain;
        presentInfo.pImageIndices = &CurrentImage;
        VkResult presentResult = vkQueuePresentKHR(vkQueue, &presentInfo);
        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
            NeedsResize = true;

        CurrentFrame++;
        loopCount++;
    }

    // Cleanup
    if (vkDevice) vkDeviceWaitIdle(vkDevice);
    pMovie.Clear();
    pMovieDef.Clear();
#ifdef GFX_ENABLE_VIDEO
    pVideo.Clear();
#endif
    delete pLoader;
    pLoader = nullptr;
    delete pCommandQueue;
    pCommandQueue = nullptr;
    pRenderer.Clear();
    if (pHAL) { pHAL->ShutdownHAL(); pHAL.Clear(); }
    CleanupVulkan();
#if defined(SF_BUILD_DEBUG) || !defined(NDEBUG)
    if (vkLogFile) { fclose(vkLogFile); vkLogFile = nullptr; }
#endif
    return 0;
}
