/**************************************************************************

Filename    :   Platform_Vulkan.cpp
Content     :   Platform Device Support for Vulkan
Created     :   April 2026
Authors     :   MonstroFil, Claude

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#define VK_USE_PLATFORM_WIN32_KHR

#include "Render/Vulkan/Vulkan_HAL.h"
#include "Platform.h"

namespace Scaleform { namespace Platform {

class DeviceImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:
    DeviceImpl(Render::ThreadCommandQueue* commandQueue);
    ~DeviceImpl();

    VkInstance               vkInstance   = VK_NULL_HANDLE;
    VkPhysicalDevice         vkPhysDevice = VK_NULL_HANDLE;
    VkDevice                 vkDevice     = VK_NULL_HANDLE;
    VkQueue                  vkQueue      = VK_NULL_HANDLE;
    uint32_t                 vkQueueFamily = 0;
    VkSurfaceKHR             vkSurface    = VK_NULL_HANDLE;
    VkSwapchainKHR           vkSwapchain  = VK_NULL_HANDLE;
    VkFormat                 vkSwapFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkExtent2D               vkSwapExtent = { 0, 0 };
    VkRenderPass             vkRenderPass       = VK_NULL_HANDLE;
    VkRenderPass             vkResumeRenderPass = VK_NULL_HANDLE; // LOAD_OP_LOAD, for post-filter restart
    VkCommandPool            vkCmdPool    = VK_NULL_HANDLE;

    const unsigned           MaxFramesInFlight = 2;
    VkCommandBuffer          vkCmdBuffers[MaxFramesInFlight] = {};
    VkSemaphore              vkImageAvailable[MaxFramesInFlight] = {};
    VkSemaphore              vkRenderFinished[MaxFramesInFlight] = {};
    VkFence                  vkInFlightFences[MaxFramesInFlight] = {};

    VkImage                  vkSwapImages[8] = {};
    VkImageView              vkSwapViews[8]  = {};
    VkFramebuffer            vkSwapFBs[8]    = {};
    uint32_t                 vkSwapCount     = 0;

    VkImage                  vkDepthImage  = VK_NULL_HANDLE;
    VkImageView              vkDepthView   = VK_NULL_HANDLE;
    VkDeviceMemory           vkDepthMemory = VK_NULL_HANDLE;
    VkFormat                 vkDepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

    uint32_t                 CurrentFrame  = 0;
    uint32_t                 CurrentImage  = 0;

    FILE* vkLogFile = nullptr;
    int   MaxFrames = -1;   // -1 = run until window close; set via --frames N

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
}

DeviceImpl::DeviceImpl(Render::ThreadCommandQueue *commandQueue)
 : pWindow(0), pHal(0), Status(Device_NeedInit),
   hWnd(0), severity(0), type(0), pData(0), pUser(0)
{
        pHal = *SF_NEW Render::VULKAN::ProfilerHAL(commandQueue);
}

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

bool DeviceImpl::adjustViewConfig(ViewConfig* config)
{

}

bool DeviceImpl::initGraphics(const ViewConfig& config, Device::Window* window,
                              ThreadId renderThreadId)
{

}

