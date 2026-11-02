/**************************************************************************

Filename    :   Platform_Vulkan.cpp
Content     :   Platform Device Support for Vulkan.
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
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_SysFile.h"
#include "Render/Render_DebugEnv.h"
#include "Render/ImageFiles/PNG_ImageFile.h"

#include <string.h>

namespace Scaleform { namespace Platform {

class DeviceImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:
    DeviceImpl(Render::ThreadCommandQueue* commandQueue);
    ~DeviceImpl();

    bool adjustViewConfig(ViewConfig* config);
    bool initGraphics(const ViewConfig& config, Device::Window* window, ThreadId renderThreadId = 0);
    bool reconfigureGraphics(const ViewConfig& config);
    void shutdownGraphics();

    bool createInstance(const ViewConfig& config);
    void destroyInstance();
    bool pickPhysicalDevice();
    bool createSurface(const ViewConfig& config);
    bool createLogicalDevice(const ViewConfig& config);
    bool createSwapchain(const ViewConfig& config);
    void destroySwapchain();
    bool createRenderPasses();
    void destroyRenderPasses();
    bool createDepthResources();
    void destroyDepthResources();
    bool createCommandPoolAndBuffers();
    void destroyCommandPoolAndBuffers();
    bool createSyncObjects();
    void destroySyncObjects();
    bool createFramebuffers();
    void destroyFramebuffers();

    void beginFrame();
    void presentFrame();
    bool takeScreenShot(const String& filename);

    Device::Window*             pWindow;
    Ptr<Render::Vulkan::HAL>    pHal;
    DeviceStatus                Status;

    HWND            hWnd;
    ThreadId        RenderThreadId;
    bool            VSync;

    // Background clear color for the swapchain image, set by Device::Clear
    // (called every frame by the render thread). Defaults to the app's gray.
    UInt32          ClearColor = 0xFF555555;

    VkInstance               vkInstance   = VK_NULL_HANDLE;
    VkPhysicalDevice         vkPhysDevice = VK_NULL_HANDLE;
    VkDevice                 vkDevice     = VK_NULL_HANDLE;
    VkQueue                  vkQueue      = VK_NULL_HANDLE;
    uint32_t                 vkQueueFamily = 0;
    VkSurfaceKHR             vkSurface    = VK_NULL_HANDLE;
    VkSwapchainKHR           vkSwapchain  = VK_NULL_HANDLE;
    VkSwapchainKHR           vkOldSwapchain = VK_NULL_HANDLE;
    VkFormat                 vkSwapFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkColorSpaceKHR          vkSwapColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    VkExtent2D               vkSwapExtent = { 0, 0 };
    VkRenderPass             vkRenderPass       = VK_NULL_HANDLE;
    VkRenderPass             vkResumeRenderPass = VK_NULL_HANDLE; // LOAD_OP_LOAD, for post-filter restart
    VkCommandPool            vkCmdPool    = VK_NULL_HANDLE;

    static const unsigned   MaxFramesInFlight = 2;
    VkCommandBuffer          vkCmdBuffers[MaxFramesInFlight] = {};
    VkSemaphore              vkImageAvailable[MaxFramesInFlight] = {};
    // Render-finished semaphores are indexed by the acquired swapchain image
    // (not the frame slot): with more swapchain images than frames in flight a
    // per-slot semaphore can be re-signaled while a presentation still holds it.
    VkSemaphore              vkRenderFinished[8] = {};
    uint32_t                 vkRenderFinishedCount = 0;
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
    bool                     FrameRecorded = false;   // true only while this frame's command buffer is recording
    bool                     ImageAcquired = false;

    #if defined(SF_BUILD_DEBUG) || !defined(NDEBUG)
    VkDebugUtilsMessengerEXT vkDebugMessenger = VK_NULL_HANDLE;

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
                            || (pData->messageIdNumber != 0 && pData->pMessageIdName && strstr(pData->pMessageIdName, "BestPractices") != nullptr);
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
        return VK_FALSE;
    }
    #endif
};

DeviceImpl::DeviceImpl(Render::ThreadCommandQueue *commandQueue)
 : pWindow(0), pHal(0), Status(Device_NeedInit),
   hWnd(0), RenderThreadId(0), VSync(false)
{
    pHal = *SF_NEW Render::Vulkan::HAL(commandQueue);
}

DeviceImpl::~DeviceImpl()
{
    shutdownGraphics();
}

bool DeviceImpl::adjustViewConfig(ViewConfig* config)
{
    // Vulkan backend does not support stereo.
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
    return true;
}

bool DeviceImpl::createInstance(const ViewConfig& config)
{
    SF_UNUSED(config);

    VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName   = "Scaleform GFx Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "Scaleform";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    // The HAL flips Y with a negative VkViewport height (see HAL::updateViewport) to
    // match the D3D/GL clip-space convention. That is only legal from Vulkan 1.1 on
    // (or with VK_KHR_maintenance1); declaring 1.0 made the validation layer report
    // VUID-VkViewport-apiVersion-07917 for every viewport and leaves the result
    // undefined on drivers that do not tolerate it.
    uint32_t loaderVersion = VK_API_VERSION_1_0;
    PFN_vkEnumerateInstanceVersion pfnEnumerateInstanceVersion =
        (PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr(VK_NULL_HANDLE,
                                                              "vkEnumerateInstanceVersion");
    if (pfnEnumerateInstanceVersion)
        pfnEnumerateInstanceVersion(&loaderVersion);
    appInfo.apiVersion = (loaderVersion >= VK_API_VERSION_1_1) ? VK_API_VERSION_1_1
                                                               : VK_API_VERSION_1_0;

    const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#if defined(SF_BUILD_DEBUG) || !defined(NDEBUG)
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
    };

    const char* layers[] = {
#if defined(SF_BUILD_DEBUG) || !defined(NDEBUG)
        "VK_LAYER_KHRONOS_validation",
#endif
        (const char*)0
    };

    VkInstanceCreateInfo instCI = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instCI.pApplicationInfo        = &appInfo;
    instCI.enabledExtensionCount   = (uint32_t)(sizeof(extensions) / sizeof(extensions[0]));
    instCI.ppEnabledExtensionNames = extensions;
#if defined(SF_BUILD_DEBUG) || !defined(NDEBUG)
    instCI.enabledLayerCount   = 1;
    instCI.ppEnabledLayerNames = layers;
#else
    SF_UNUSED(layers);
    instCI.enabledLayerCount = 0;
#endif

    if (vkCreateInstance(&instCI, nullptr, &vkInstance) != VK_SUCCESS)
    {
        SF_DEBUG_WARNING(1, "Vulkan: Failed to create instance.");
        return false;
    }

#if defined(SF_BUILD_DEBUG) || !defined(NDEBUG)
    // Set up the debug messenger.
    auto pfnCreateDebugMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");
    if (pfnCreateDebugMessenger)
    {
        VkDebugUtilsMessengerCreateInfoEXT dbgCI = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        dbgCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dbgCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        dbgCI.pfnUserCallback = vkDebugCallback;
        dbgCI.pUserData       = 0;
        pfnCreateDebugMessenger(vkInstance, &dbgCI, nullptr, &vkDebugMessenger);
    }
#endif
    return true;
}

void DeviceImpl::destroyInstance()
{
#if defined(SF_BUILD_DEBUG) || !defined(NDEBUG)
    if (vkDebugMessenger != VK_NULL_HANDLE)
    {
        auto pfnDestroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT");
        if (pfnDestroyDebugMessenger)
            pfnDestroyDebugMessenger(vkInstance, vkDebugMessenger, nullptr);
        vkDebugMessenger = VK_NULL_HANDLE;
    }
#endif
    if (vkInstance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(vkInstance, nullptr);
        vkInstance = VK_NULL_HANDLE;
    }
}

bool DeviceImpl::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    if (vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr) != VK_SUCCESS || deviceCount == 0)
        return false;

    VkPhysicalDevice devices[8];
    deviceCount = deviceCount > 8 ? 8 : deviceCount;
    if (vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices) != VK_SUCCESS)
        return false;

    for (uint32_t i = 0; i < deviceCount; i++)
    {
        // Check for a graphics queue family that also supports presentation.
        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &familyCount, nullptr);
        if (familyCount == 0)
            continue;
        VkQueueFamilyProperties families[16];
        familyCount = familyCount > 16 ? 16 : familyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &familyCount, families);

        for (uint32_t f = 0; f < familyCount; f++)
        {
            if (!(families[f].queueFlags & VK_QUEUE_GRAPHICS_BIT))
                continue;
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], f, vkSurface, &presentSupport);
            if (presentSupport)
            {
                vkPhysDevice = devices[i];
                vkQueueFamily = f;
                return true;
            }
        }
    }
    return false;
}

bool DeviceImpl::createSurface(const ViewConfig& config)
{
    SF_UNUSED(config);
    VkWin32SurfaceCreateInfoKHR surfCI = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    surfCI.hinstance = GetModuleHandle(0);
    surfCI.hwnd      = hWnd;
    return vkCreateWin32SurfaceKHR(vkInstance, &surfCI, nullptr, &vkSurface) == VK_SUCCESS;
}

bool DeviceImpl::createLogicalDevice(const ViewConfig& config)
{
    SF_UNUSED(config);

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCI = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queueCI.queueFamilyIndex = vkQueueFamily;
    queueCI.queueCount       = 1;
    queueCI.pQueuePriorities = &queuePriority;

    const char* extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkDeviceCreateInfo devCI = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    devCI.queueCreateInfoCount    = 1;
    devCI.pQueueCreateInfos       = &queueCI;
    devCI.enabledExtensionCount   = 1;
    devCI.ppEnabledExtensionNames = extensions;

    if (vkCreateDevice(vkPhysDevice, &devCI, nullptr, &vkDevice) != VK_SUCCESS)
        return false;

    vkGetDeviceQueue(vkDevice, vkQueueFamily, 0, &vkQueue);
    return true;
}

bool DeviceImpl::createSwapchain(const ViewConfig& config)
{
    // Query surface capabilities.
    VkSurfaceCapabilitiesKHR caps;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysDevice, vkSurface, &caps) != VK_SUCCESS)
        return false;

    // Choose surface format.
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysDevice, vkSurface, &formatCount, nullptr);
    VkSurfaceFormatKHR formats[32];
    formatCount = formatCount > 32 ? 32 : formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysDevice, vkSurface, &formatCount, formats);
    if (formatCount == 0)
        return false;

    vkSwapFormat = formats[0].format;
    vkSwapColorSpace = formats[0].colorSpace;
    for (uint32_t i = 0; i < formatCount; i++)
    {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM)
        {
            vkSwapFormat = formats[i].format;
            vkSwapColorSpace = formats[i].colorSpace;
            break;
        }
    }

    // Choose present mode: FIFO always available; use MAILBOX when vsync is off.
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!VSync)
    {
        uint32_t modeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysDevice, vkSurface, &modeCount, nullptr);
        VkPresentModeKHR modes[16];
        modeCount = modeCount > 16 ? 16 : modeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysDevice, vkSurface, &modeCount, modes);
        for (uint32_t i = 0; i < modeCount; i++)
        {
            if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
        }
    }

    // Choose extent.
    if (caps.currentExtent.width != 0xFFFFFFFF)
        vkSwapExtent = caps.currentExtent;
    else
    {
        vkSwapExtent.width  = config.ViewSize.Width;
        vkSwapExtent.height = config.ViewSize.Height;
        vkSwapExtent.width  = vkSwapExtent.width  < caps.minImageExtent.width  ? caps.minImageExtent.width  :
                              vkSwapExtent.width  > caps.maxImageExtent.width  ? caps.maxImageExtent.width  : vkSwapExtent.width;
        vkSwapExtent.height = vkSwapExtent.height < caps.minImageExtent.height ? caps.minImageExtent.height :
                              vkSwapExtent.height > caps.maxImageExtent.height ? caps.maxImageExtent.height : vkSwapExtent.height;
    }

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
        imageCount = caps.maxImageCount;

    VkSwapchainCreateInfoKHR swapCI = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    swapCI.surface            = vkSurface;
    swapCI.minImageCount      = imageCount;
    swapCI.imageFormat        = vkSwapFormat;
    swapCI.imageColorSpace    = vkSwapColorSpace;
    swapCI.imageExtent        = vkSwapExtent;
    swapCI.imageArrayLayers   = 1;
    swapCI.imageUsage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    swapCI.imageSharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    swapCI.preTransform       = caps.currentTransform;
    swapCI.compositeAlpha     = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapCI.presentMode        = presentMode;
    swapCI.clipped            = VK_TRUE;
    swapCI.oldSwapchain       = vkOldSwapchain;

    if (vkCreateSwapchainKHR(vkDevice, &swapCI, nullptr, &vkSwapchain) != VK_SUCCESS)
        return false;

    if (vkOldSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(vkDevice, vkOldSwapchain, nullptr);
        vkOldSwapchain = VK_NULL_HANDLE;
    }

    // Get the swap images.
    vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &vkSwapCount, nullptr);
    if (vkSwapCount > 8)
        vkSwapCount = 8;
    if (vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &vkSwapCount, vkSwapImages) != VK_SUCCESS)
        return false;

    // Create image views.
    for (uint32_t i = 0; i < vkSwapCount; i++)
    {
        VkImageViewCreateInfo viewCI = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewCI.image            = vkSwapImages[i];
        viewCI.viewType         = VK_IMAGE_VIEW_TYPE_2D;
        viewCI.format           = vkSwapFormat;
        viewCI.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCI.subresourceRange.baseMipLevel   = 0;
        viewCI.subresourceRange.levelCount     = 1;
        viewCI.subresourceRange.baseArrayLayer = 0;
        viewCI.subresourceRange.layerCount     = 1;
        if (vkCreateImageView(vkDevice, &viewCI, nullptr, &vkSwapViews[i]) != VK_SUCCESS)
            return false;
    }
    return true;
}

void DeviceImpl::destroySwapchain()
{
    for (uint32_t i = 0; i < vkSwapCount; i++)
    {
        if (vkSwapFBs[i] != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(vkDevice, vkSwapFBs[i], nullptr);
            vkSwapFBs[i] = VK_NULL_HANDLE;
        }
        if (vkSwapViews[i] != VK_NULL_HANDLE)
        {
            vkDestroyImageView(vkDevice, vkSwapViews[i], nullptr);
            vkSwapViews[i] = VK_NULL_HANDLE;
        }
        vkSwapImages[i] = VK_NULL_HANDLE;
    }
    vkSwapCount = 0;
    if (vkSwapchain != VK_NULL_HANDLE)
    {
        if (vkDevice != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr);
            vkSwapchain = VK_NULL_HANDLE;
        }
    }
}

bool DeviceImpl::createRenderPasses()
{
    // Color attachment: swapchain image.
    VkAttachmentDescription colorAttach = {};
    colorAttach.format         = vkSwapFormat;
    colorAttach.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttach.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttach.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttach.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // The main pass leaves the swapchain image in COLOR_ATTACHMENT_OPTIMAL so
    // that it can be resumed by vkResumeRenderPass below (which expects that
    // layout) after the HAL bounces through offscreen filter targets.  The
    // transition to PRESENT_SRC_KHR is emitted explicitly in presentFrame(),
    // right before the command buffer is submitted.
    colorAttach.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttach.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth attachment.
    //
    // Depth/stencil must be STORED, not discarded: the HAL ends this pass and resumes it
    // through vkResumeRenderPass whenever a filter bounces through an offscreen target.
    // With DONT_CARE the contents are undefined at the pass boundary, so masks (stencil
    // bits) written before the filter were lost and everything drawn into them after the
    // resume - e.g. the check-mark animation on 9-1-B - was clipped away.
    VkAttachmentDescription depthAttach = {};
    depthAttach.format         = vkDepthFormat;
    depthAttach.samples        = VK_SAMPLE_COUNT_1_BIT;
    depthAttach.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttach.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttach.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttach.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttach.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef = {};
    depthRef.attachment = 1;
    depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;

    VkSubpassDependency dep = {};
    dep.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass    = 0;
    dep.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dep.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dep.srcAccessMask = 0;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[2] = { colorAttach, depthAttach };
    VkRenderPassCreateInfo rpCI = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    rpCI.attachmentCount = 2;
    rpCI.pAttachments    = attachments;
    rpCI.subpassCount    = 1;
    rpCI.pSubpasses      = &subpass;
    rpCI.dependencyCount = 1;
    rpCI.pDependencies   = &dep;
    if (vkCreateRenderPass(vkDevice, &rpCI, nullptr, &vkRenderPass) != VK_SUCCESS)
        return false;

    // Resume render pass: LOAD_OP_LOAD so the main pass can be restarted after
    // an offscreen filter pass without clearing what was already drawn.  Its
    // initialLayout matches the finalLayout of the main pass, so resuming after
    // the HAL ends the main pass requires no explicit image layout transition.
    VkAttachmentDescription resumeColor = colorAttach;
    resumeColor.loadOp        = VK_ATTACHMENT_LOAD_OP_LOAD;
    resumeColor.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth/stencil survives the bounce as well, so stencil masks set up before a
    // filter still clip the content drawn after it (see the depthAttach comment).
    VkAttachmentDescription resumeDepth = depthAttach;
    resumeDepth.loadOp        = VK_ATTACHMENT_LOAD_OP_LOAD;
    resumeDepth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    resumeDepth.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription resumeAttachments[2] = { resumeColor, resumeDepth };
    VkRenderPassCreateInfo resumeCI = rpCI;
    resumeCI.pAttachments    = resumeAttachments;
    if (vkCreateRenderPass(vkDevice, &resumeCI, nullptr, &vkResumeRenderPass) != VK_SUCCESS)
        return false;

    return true;
}

void DeviceImpl::destroyRenderPasses()
{
    if (vkResumeRenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(vkDevice, vkResumeRenderPass, nullptr);
        vkResumeRenderPass = VK_NULL_HANDLE;
    }
    if (vkRenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
        vkRenderPass = VK_NULL_HANDLE;
    }
}

bool DeviceImpl::createDepthResources()
{
    VkImageCreateInfo imgCI = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imgCI.imageType     = VK_IMAGE_TYPE_2D;
    imgCI.extent.width  = vkSwapExtent.width;
    imgCI.extent.height = vkSwapExtent.height;
    imgCI.extent.depth  = 1;
    imgCI.mipLevels     = 1;
    imgCI.arrayLayers   = 1;
    imgCI.format        = vkDepthFormat;
    imgCI.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imgCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgCI.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imgCI.samples       = VK_SAMPLE_COUNT_1_BIT;
    imgCI.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(vkDevice, &imgCI, nullptr, &vkDepthImage) != VK_SUCCESS)
        return false;

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(vkDevice, vkDepthImage, &memReqs);

    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize  = memReqs.size;
    allocInfo.memoryTypeIndex = Scaleform::Render::Vulkan::FindMemoryType(vkPhysDevice,
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (allocInfo.memoryTypeIndex == UINT32_MAX ||
        vkAllocateMemory(vkDevice, &allocInfo, nullptr, &vkDepthMemory) != VK_SUCCESS)
        return false;
    vkBindImageMemory(vkDevice, vkDepthImage, vkDepthMemory, 0);

    VkImageViewCreateInfo viewCI = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewCI.image            = vkDepthImage;
    viewCI.viewType         = VK_IMAGE_VIEW_TYPE_2D;
    viewCI.format           = vkDepthFormat;
    viewCI.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    viewCI.subresourceRange.baseMipLevel   = 0;
    viewCI.subresourceRange.levelCount     = 1;
    viewCI.subresourceRange.baseArrayLayer = 0;
    viewCI.subresourceRange.layerCount     = 1;
    return vkCreateImageView(vkDevice, &viewCI, nullptr, &vkDepthView) == VK_SUCCESS;
}

void DeviceImpl::destroyDepthResources()
{
    if (vkDepthView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(vkDevice, vkDepthView, nullptr);
        vkDepthView = VK_NULL_HANDLE;
    }
    if (vkDepthImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(vkDevice, vkDepthImage, nullptr);
        vkDepthImage = VK_NULL_HANDLE;
    }
    if (vkDepthMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(vkDevice, vkDepthMemory, nullptr);
        vkDepthMemory = VK_NULL_HANDLE;
    }
}

bool DeviceImpl::createFramebuffers()
{
    for (uint32_t i = 0; i < vkSwapCount; i++)
    {
        VkImageView attachments[2] = { vkSwapViews[i], vkDepthView };
        VkFramebufferCreateInfo fbCI = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        fbCI.renderPass      = vkRenderPass;
        fbCI.attachmentCount = 2;
        fbCI.pAttachments    = attachments;
        fbCI.width           = vkSwapExtent.width;
        fbCI.height          = vkSwapExtent.height;
        fbCI.layers          = 1;
        if (vkCreateFramebuffer(vkDevice, &fbCI, nullptr, &vkSwapFBs[i]) != VK_SUCCESS)
            return false;    }
    return true;
}

void DeviceImpl::destroyFramebuffers()
{
    for (uint32_t i = 0; i < vkSwapCount; i++)
    {
        if (vkSwapFBs[i] != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(vkDevice, vkSwapFBs[i], nullptr);
            vkSwapFBs[i] = VK_NULL_HANDLE;
        }
    }
}

bool DeviceImpl::createCommandPoolAndBuffers()
{
    VkCommandPoolCreateInfo poolCI = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    poolCI.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCI.queueFamilyIndex = vkQueueFamily;
    if (vkCreateCommandPool(vkDevice, &poolCI, nullptr, &vkCmdPool) != VK_SUCCESS)
        return false;

    VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool        = vkCmdPool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MaxFramesInFlight;
    return vkAllocateCommandBuffers(vkDevice, &allocInfo, vkCmdBuffers) == VK_SUCCESS;
}

void DeviceImpl::destroyCommandPoolAndBuffers()
{
    if (vkCmdPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(vkDevice, vkCmdPool, nullptr);
        vkCmdPool = VK_NULL_HANDLE;
    }
    for (unsigned i = 0; i < MaxFramesInFlight; i++)
        vkCmdBuffers[i] = VK_NULL_HANDLE;
}

bool DeviceImpl::createSyncObjects()
{
    VkSemaphoreCreateInfo semCI = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceCI = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // Swapchain images are created before sync objects, so vkSwapCount is known.
    // Create one render-finished semaphore per swapchain image (indexed by the
    // acquired image at submit/present time) and one acquire semaphore + fence
    // per frame slot.
    vkRenderFinishedCount = (vkSwapCount <= 8) ? vkSwapCount : 8;
    for (unsigned i = 0; i < vkRenderFinishedCount; i++)
    {
        if (vkCreateSemaphore(vkDevice, &semCI, nullptr, &vkRenderFinished[i]) != VK_SUCCESS)
            return false;
    }
    for (unsigned i = 0; i < MaxFramesInFlight; i++)
    {
        if (vkCreateSemaphore(vkDevice, &semCI, nullptr, &vkImageAvailable[i]) != VK_SUCCESS ||
            vkCreateFence(vkDevice, &fenceCI, nullptr, &vkInFlightFences[i]) != VK_SUCCESS)
            return false;
    }
    return true;
}

void DeviceImpl::destroySyncObjects()
{
    for (unsigned i = 0; i < vkRenderFinishedCount; i++)
    {
        if (vkRenderFinished[i] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(vkDevice, vkRenderFinished[i], nullptr);
            vkRenderFinished[i] = VK_NULL_HANDLE;
        }
    }
    vkRenderFinishedCount = 0;
    for (unsigned i = 0; i < MaxFramesInFlight; i++)
    {
        if (vkInFlightFences[i] != VK_NULL_HANDLE)
        {
            vkDestroyFence(vkDevice, vkInFlightFences[i], nullptr);
            vkInFlightFences[i] = VK_NULL_HANDLE;
        }
        if (vkImageAvailable[i] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(vkDevice, vkImageAvailable[i], nullptr);
            vkImageAvailable[i] = VK_NULL_HANDLE;
        }
    }
}

bool DeviceImpl::initGraphics(const ViewConfig& config, Device::Window* window,
                              ThreadId renderThreadId)
{
    if (!pHal || !window)
        return false;

    pWindow         = window;
    hWnd            = (HWND)window->GetHandle();
    RenderThreadId  = renderThreadId;
    VSync           = config.HasFlag(View_VSync);

    if (!createInstance(config))  return false;
    if (!createSurface(config))   return false;
    if (!pickPhysicalDevice())    return false;
    if (!createLogicalDevice(config)) return false;
    if (!createSwapchain(config)) return false;
    if (!createRenderPasses())    return false;
    if (!createDepthResources())  return false;
    if (!createFramebuffers())    return false;
    if (!createCommandPoolAndBuffers()) return false;
    if (!createSyncObjects())     return false;

    Render::Vulkan::HALInitParams params(vkDevice, vkPhysDevice, vkQueue, vkQueueFamily,
                                         vkCmdPool, vkCmdBuffers[0], vkRenderPass,
                                         vkSwapFormat, vkInstance, 0, renderThreadId);
    if (!pHal->InitHAL(params))
    {
        shutdownGraphics();
        pWindow = 0;
        hWnd = 0;
        return false;
    }

    // Attach the initial swapchain framebuffer to the HAL.
    pHal->SetMainRenderTarget(vkRenderPass, vkResumeRenderPass, vkSwapFBs[0], vkSwapExtent);

    Status = Device_Ready;
    return true;
}

bool DeviceImpl::reconfigureGraphics(const ViewConfig& config)
{
    if (!vkDevice || !vkSwapchain)
        return false;

    static bool reconfigureActive = false;
    if (reconfigureActive)
        return true;
    reconfigureActive = true;

    vkDeviceWaitIdle(vkDevice);

    VSync = config.HasFlag(View_VSync);

    // Recreate the swapchain and dependent resources.
    vkOldSwapchain = vkSwapchain;
    vkSwapchain = VK_NULL_HANDLE;

    destroyFramebuffers();
    destroyDepthResources();

    if (!createSwapchain(config))
    {
        reconfigureActive = false;
        return false;
    }
    if (!createDepthResources() || !createFramebuffers())
    {
        reconfigureActive = false;
        return false;
    }

    pHal->SetMainRenderTarget(vkRenderPass, vkResumeRenderPass, vkSwapFBs[0], vkSwapExtent);
    Status = Device_Ready;
    reconfigureActive = false;
    return true;
}

void DeviceImpl::shutdownGraphics()
{
    if (!vkDevice)
        return;

    if (vkSwapchain != VK_NULL_HANDLE)
        vkDeviceWaitIdle(vkDevice);

    if (pHal)
        pHal->ShutdownHAL();

    destroyFramebuffers();
    destroySyncObjects();
    destroyCommandPoolAndBuffers();
    destroyDepthResources();
    destroyRenderPasses();
    destroySwapchain();
    if (vkOldSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(vkDevice, vkOldSwapchain, nullptr);
        vkOldSwapchain = VK_NULL_HANDLE;
    }
    if (vkSurface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
        vkSurface = VK_NULL_HANDLE;
    }
    if (vkDevice != VK_NULL_HANDLE)
    {
        vkDestroyDevice(vkDevice, nullptr);
        vkDevice = VK_NULL_HANDLE;
    }
    vkPhysDevice = VK_NULL_HANDLE;
    destroyInstance();

    pWindow = 0;
    hWnd = 0;
    Status = Device_NeedInit;
}

void DeviceImpl::beginFrame()
{
    // Nothing is recorded until acquisition and command-buffer begin succeed.
    FrameRecorded = false;

    if (!vkDevice || !vkSwapchain)
        return;

    uint32_t frame = CurrentFrame % MaxFramesInFlight;

    // Wait for the in-flight fence of this frame slot.
    vkWaitForFences(vkDevice, 1, &vkInFlightFences[frame], VK_TRUE, UINT64_MAX);

    // The GPU is finished with this frame slot: now it is safe to reclaim the
    // ImageViews/framebuffers/textures queued for it. Doing this in HAL::BeginScene
    // ran before this wait and destroyed objects still referenced by in-flight
    // command buffers (validation: "VkImageView ... was destroyed").
    if (pHal)
        pHal->DrainFrameResources();

    // Acquire the next swapchain image.
    VkResult r = vkAcquireNextImageKHR(vkDevice, vkSwapchain, UINT64_MAX,
                                       vkImageAvailable[frame], VK_NULL_HANDLE, &CurrentImage);
    if (r == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // The swapchain is out of date (the window was resized). Rebuild it now
        // so the next frame can acquire and record; presentFrame() skips this
        // frame because nothing was recorded into the command buffer.
        ViewConfig config;
        config.SetSize(vkSwapExtent.width, vkSwapExtent.height);
        reconfigureGraphics(config);
        return;
    }
    if (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR)
        return;
    ImageAcquired = true;
    static const bool sImgTrace = Render::IsDebugEnvSet("GFX_VKIMG");
    if (sImgTrace)
        fprintf(stderr, "[VKIMG] begin img=%u fb=%p\n", CurrentImage, (void*)vkSwapFBs[CurrentImage]), fflush(stderr);

    vkResetFences(vkDevice, 1, &vkInFlightFences[frame]);

    // Tell the renderer which VkFence signals the end of *this* frame.
    //
    // Vulkan::RenderSync hands this fence out (SetFence()) for every GPU resource
    // tagged while the frame is being recorded, and IsPending() polls it to decide
    // when a buffer/MeshCacheItem can be recycled. The contract (see
    // Vulkan_Sync.h) is that the platform layer supplies it right here, before
    // HAL::BeginFrame() runs. Without this call ExternalFence stays NULL, every
    // fence appears already complete and the mesh cache recycles vertex/index
    // buffers and MeshCacheItem objects - which live in the Scaleform heap - while
    // the GPU and the render tree are still using them. That produced the random
    // heap corruption crashes reported in StateBag/FilterSet/TreeCache code.
    pHal->GetMeshCache().GetRenderSync()->SetExternalFence(vkInFlightFences[frame]);

    // Begin the command buffer and the main render pass.
    vkResetCommandBuffer(vkCmdBuffers[frame], 0);
    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(vkCmdBuffers[frame], &beginInfo) != VK_SUCCESS)
        return;

    VkClearValue clearVals[2];
    memset(clearVals, 0, sizeof(clearVals));

    // Diagnostic clear-color override (VKFILTER E1): GFX_VK_CLEAR=RRGGBB.
    // A saturated background makes it immediately obvious whether a frame wrote any
    // pixels at all, and shows the exact extent of whatever is painted over them.
    // Parsed once; the normal path keeps using the app-provided ClearColor.
    static bool   sClearOverrideInit = false;
    static UInt32 sClearOverride     = 0;
    if (!sClearOverrideInit)
    {
        sClearOverrideInit = true;
        char clearEnv[32];
        if (Render::GetDebugEnvValue("GFX_VK_CLEAR", clearEnv, sizeof(clearEnv)))
            sClearOverride = ((UInt32)strtoul(clearEnv, 0, 16) & 0xFFFFFFu) | 0xFF000000u;
    }
    const UInt32 clearColor = sClearOverride ? sClearOverride : ClearColor;

    // Clear the swapchain image to the app's background color (0xFF555555 gray
    // by default), matching the D3D9 platform device's per-frame clear.
    clearVals[0].color.float32[0] = (float)((clearColor >> 16) & 0xFF) / 255.0f;
    clearVals[0].color.float32[1] = (float)((clearColor >> 8) & 0xFF) / 255.0f;
    clearVals[0].color.float32[2] = (float)(clearColor & 0xFF) / 255.0f;
    clearVals[0].color.float32[3] = (float)((clearColor >> 24) & 0xFF) / 255.0f;
    clearVals[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo rpBI = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    rpBI.renderPass  = vkRenderPass;
    rpBI.framebuffer = vkSwapFBs[CurrentImage];
    rpBI.renderArea.offset = { 0, 0 };
    rpBI.renderArea.extent = vkSwapExtent;
    rpBI.clearValueCount = 2;
    rpBI.pClearValues    = clearVals;
    vkCmdBeginRenderPass(vkCmdBuffers[frame], &rpBI, VK_SUBPASS_CONTENTS_INLINE);

    // Hand the command buffer + current framebuffer to the HAL.
    pHal->SetCommandBuffer(vkCmdBuffers[frame]);
    pHal->SetFrameSlot(frame);
    pHal->SetMainRenderTarget(vkRenderPass, vkResumeRenderPass, vkSwapFBs[CurrentImage], vkSwapExtent);
    pHal->SetMainPassActive(true);   // marks the swapchain pass as recording
    FrameRecorded = true;
}

void DeviceImpl::presentFrame()
{
    if (!vkDevice || !vkSwapchain)
        return;

    // beginFrame() may have skipped this frame (swapchain out of date,
    // acquisition/recording failure, or a resize-triggered present without a
    // matching BeginFrame). There is no command buffer to end or submit, and
    // calling vkCmdEndRenderPass/vkEndCommandBuffer on a non-recording buffer
    // violates the Vulkan spec (validation errors + crash).
    if (!FrameRecorded)
    {
        // The image acquired in beginFrame() must be returned even when nothing was
        // recorded, otherwise the swapchain runs dry and the render-finished
        // semaphore is never waited on ("previously signaled ... not waited on").
        if (ImageAcquired && vkQueue)
        {
            VkPresentInfoKHR pi = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
            pi.swapchainCount = 1;
            pi.pSwapchains    = &vkSwapchain;
            pi.pImageIndices  = &CurrentImage;
            vkQueuePresentKHR(vkQueue, &pi);
            ImageAcquired = false;
        }
        CurrentFrame++;
        return;
    }

    // The frame is being ended now; a second PresentFrame without a new
    // BeginFrame (e.g. the resize-triggered present-on-resize path) must no-op.
    FrameRecorded = false;

    uint32_t frame = CurrentFrame % MaxFramesInFlight;

    // End the main render pass.  The HAL ends and restarts the swapchain pass
    // around offscreen filter targets, so it owns the authoritative state:
    // blindly calling vkCmdEndRenderPass here would be a validation error (and
    // undefined behaviour) whenever the HAL left a filter pass open.
    if (pHal->InRenderPass)
        vkCmdEndRenderPass(vkCmdBuffers[frame]);
    pHal->SetMainPassActive(false);   // clears both MainPassActive and InRenderPass

    // Hand the image back to the presentation engine.  The swapchain pass ends
    // in COLOR_ATTACHMENT_OPTIMAL (its finalLayout) so the HAL can resume it;
    // the transition to PRESENT_SRC_KHR is emitted once here, before submit.
    {
        VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        barrier.oldLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image         = vkSwapImages[CurrentImage];
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;
        vkCmdPipelineBarrier(vkCmdBuffers[frame],
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    if (vkEndCommandBuffer(vkCmdBuffers[frame]) != VK_SUCCESS)
        return;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores    = &vkImageAvailable[frame];
    submitInfo.pWaitDstStageMask  = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &vkCmdBuffers[frame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &vkRenderFinished[CurrentImage];

    if (vkQueueSubmit(vkQueue, 1, &submitInfo, vkInFlightFences[frame]) != VK_SUCCESS)
        return;

    VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &vkRenderFinished[CurrentImage];
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &vkSwapchain;
    presentInfo.pImageIndices      = &CurrentImage;

    VkResult r = vkQueuePresentKHR(vkQueue, &presentInfo);
    static const bool sImgTrace = Render::IsDebugEnvSet("GFX_VKIMG");
    if (sImgTrace)
        fprintf(stderr, "[VKIMG] present img=%u\n", CurrentImage), fflush(stderr);
    ImageAcquired = false;
    if (r == VK_ERROR_OUT_OF_DATE_KHR || r == VK_SUBOPTIMAL_KHR)
    {
        // Swapchain needs recreation; reconfigure on the next frame.
        ViewConfig config;
        config.SetSize(vkSwapExtent.width, vkSwapExtent.height);
        reconfigureGraphics(config);
    }

    CurrentFrame++;
}

bool DeviceImpl::takeScreenShot(const String& filename)
{
    if (!vkDevice || !vkSwapchain || !vkQueue)
        return false;

    String actualFilename = filename;
    if (filename.GetExtension().CompareNoCase(".png") != 0)
        actualFilename.AppendString(".png");

    uint32_t frame = CurrentFrame % MaxFramesInFlight;
    SF_UNUSED(frame);
    VkImage srcImage = vkSwapImages[CurrentImage];

    // Copy the current swap image into a host-visible buffer.
    VkDeviceSize imageSize = (VkDeviceSize)vkSwapExtent.width * vkSwapExtent.height * 4;
	SF_UNUSED(imageSize);

    VkDeviceSize rowPitch = (vkSwapExtent.width * 4 + 255) & ~255u;
    VkDeviceSize bufferSize = rowPitch * vkSwapExtent.height;

    VkBufferCreateInfo bufCI = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCI.size = bufferSize;
    bufCI.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer readbackBuffer = VK_NULL_HANDLE;
    if (vkCreateBuffer(vkDevice, &bufCI, nullptr, &readbackBuffer) != VK_SUCCESS)
        return false;

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(vkDevice, readbackBuffer, &memReqs);
    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize  = memReqs.size;
    allocInfo.memoryTypeIndex = Scaleform::Render::Vulkan::FindMemoryType(vkPhysDevice,
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDeviceMemory readbackMemory = VK_NULL_HANDLE;
    if (allocInfo.memoryTypeIndex == UINT32_MAX ||
        vkAllocateMemory(vkDevice, &allocInfo, nullptr, &readbackMemory) != VK_SUCCESS)
    {
        vkDestroyBuffer(vkDevice, readbackBuffer, nullptr);
        return false;
    }
    vkBindBufferMemory(vkDevice, readbackBuffer, readbackMemory, 0);

    // One-off command buffer for the copy.
    VkCommandBufferAllocateInfo cbAlloc = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cbAlloc.commandPool        = vkCmdPool;
    cbAlloc.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbAlloc.commandBufferCount = 1;
    VkCommandBuffer cmd;
    if (vkAllocateCommandBuffers(vkDevice, &cbAlloc, &cmd) != VK_SUCCESS)
    {
        vkFreeMemory(vkDevice, readbackMemory, nullptr);
        vkDestroyBuffer(vkDevice, readbackBuffer, nullptr);
        return false;
    }

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    // Transition the swap image to TRANSFER_SRC.
    VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.oldLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image          = srcImage;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    VkBufferImageCopy region = {};
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0; // tightly packed per the footprint
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount     = 1;
    region.imageExtent = { vkSwapExtent.width, vkSwapExtent.height, 1 };
    vkCmdCopyImageToBuffer(cmd, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           readbackBuffer, 1, &region);

    // Transition back to PRESENT.
    barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = 0;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &cmd;
    vkQueueSubmit(vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vkQueue);
    vkFreeCommandBuffers(vkDevice, vkCmdPool, 1, &cmd);

    // Map and save as PNG.
    void* pData = 0;
    vkMapMemory(vkDevice, readbackMemory, 0, bufferSize, 0, &pData);

    Render::ImageData data;
    UInt32* pDstRow = (UInt32*)SF_ALLOC(vkSwapExtent.width * vkSwapExtent.height * 4, Stat_Default_Mem);
    for (uint32_t y = 0; y < vkSwapExtent.height; y++)
    {
        UInt32* ps = (UInt32*)((UByte*)pData + y * rowPitch);
        UInt32* pd = pDstRow + y * vkSwapExtent.width;
        for (uint32_t x = 0; x < vkSwapExtent.width; x++)
        {
            UInt32 c = ps[x];
            pd[x] = 0xFF000000 | ((c & 0x00FF0000) >> 16) | (c & 0x0000FF00) | ((c & 0x000000FF) << 16);
        }
    }
    data.Initialize(Render::Image_R8G8B8A8, vkSwapExtent.width, vkSwapExtent.height,
                    vkSwapExtent.width * 4, (UByte*)pDstRow);

#ifdef SF_ENABLE_LIBPNG
    bool success = Render::PNG::FileWriter::WriteImage<SysFile>(actualFilename, data);
#else
    bool success = false;
#endif
    SF_FREE(pDstRow);
    vkUnmapMemory(vkDevice, readbackMemory);
    vkFreeMemory(vkDevice, readbackMemory, nullptr);
    vkDestroyBuffer(vkDevice, readbackBuffer, nullptr);
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
    // Store the background color; it is used to clear the swapchain image at
    // the start of the next frame's render pass (mirrors the D3D9 platform
    // device, which clears the back buffer with this color every frame).
    pImpl->ClearColor = color;
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
    pImpl->pHal->SetRasterMode(flag ? Render::Vulkan::HAL::RasterMode_Wireframe :
        Render::Vulkan::HAL::RasterMode_Default);
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





