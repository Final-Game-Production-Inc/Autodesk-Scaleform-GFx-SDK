/**************************************************************************

Filename    :   CaptureVulkan.cpp
Content     :   Headless Vulkan frame capture tool for visual regression testing.
                Renders a SWF to specified frames and saves PPM images.

CLI:    CaptureVulkan.exe <swf_path> <output_dir> --frames 1,10,30,60
                          [--width 1024] [--height 768]

Based on GFxPlayerTinyVulkan.cpp with:
  - No interactive input, no HUD
  - Deterministic 30fps advance
  - Black background (SetBackgroundAlpha 1.0)
  - VK_IMAGE_USAGE_TRANSFER_SRC_BIT on swapchain for readback
  - Auto-exit after last capture frame

**************************************************************************/

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>

#include "GFx_Kernel.h"
#include "GFx.h"
#include "GFx/GFx_AS3Support.h"
#include "GFx/AS3/AS3_Global.h"
#include "GFx/GFx_ImageCreator.h"
#include "GFx_Renderer_Vulkan.h"
#include "Render/Renderer2D.h"
#include "Render/ImageFiles/JPEG_ImageFile.h"
#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/ImageFiles/TGA_ImageFile.h"
#include "Render/ImageFiles/DDS_ImageFile.h"
#include "GFx_FontProvider_Win32.h"

using namespace Scaleform;
using namespace Scaleform::Render;
using namespace Scaleform::GFx;
namespace SF = Scaleform;

// ---------------------------------------------------------------------------
// Globals (mirrors GFxPlayerTinyVulkan structure)
// ---------------------------------------------------------------------------

static HWND             hWnd         = NULL;
static unsigned         WindowWidth  = 1024;
static unsigned         WindowHeight = 768;

static VkInstance       vkInstance   = VK_NULL_HANDLE;
static VkPhysicalDevice vkPhysDevice = VK_NULL_HANDLE;
static VkDevice         vkDevice     = VK_NULL_HANDLE;
static VkQueue          vkQueue      = VK_NULL_HANDLE;
static uint32_t         vkQueueFamily = 0;
static VkSurfaceKHR     vkSurface    = VK_NULL_HANDLE;
static VkSwapchainKHR   vkSwapchain  = VK_NULL_HANDLE;
static VkFormat         vkSwapFormat = VK_FORMAT_B8G8R8A8_UNORM;
static VkExtent2D       vkSwapExtent = { 0, 0 };
static VkRenderPass     vkRenderPass       = VK_NULL_HANDLE;
static VkRenderPass     vkResumeRenderPass = VK_NULL_HANDLE;
static VkCommandPool    vkCmdPool    = VK_NULL_HANDLE;

static const unsigned   MaxFramesInFlight = 2;
static VkCommandBuffer  vkCmdBuffers[MaxFramesInFlight] = {};
static VkSemaphore      vkImageAvailable[MaxFramesInFlight] = {};
static VkSemaphore      vkRenderFinished[MaxFramesInFlight] = {};
static VkFence          vkInFlightFences[MaxFramesInFlight] = {};

static VkImage          vkSwapImages[8] = {};
static VkImageView      vkSwapViews[8]  = {};
static VkFramebuffer    vkSwapFBs[8]    = {};
static uint32_t         vkSwapCount     = 0;

static VkImage          vkDepthImage  = VK_NULL_HANDLE;
static VkImageView      vkDepthView   = VK_NULL_HANDLE;
static VkDeviceMemory   vkDepthMemory = VK_NULL_HANDLE;
static VkFormat         vkDepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

static uint32_t         CurrentFrame  = 0;
static uint32_t         CurrentImage  = 0;

static Ptr<Vulkan::HAL>  pHAL;
static Ptr<Renderer2D>   pRenderer;

// ---------------------------------------------------------------------------
// Silent log (headless)
// ---------------------------------------------------------------------------

class CaptureLog : public Scaleform::Log
{
public:
    virtual void LogMessageVarg(Scaleform::LogMessageId id, const char* fmt, va_list args)
    {
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
    }
};

// ---------------------------------------------------------------------------
// Thread command queue (single-threaded: execute immediately)
// ---------------------------------------------------------------------------

class CaptureThreadCommandQueue : public Render::ThreadCommandQueue
{
public:
    Vulkan::HAL* pHAL;
    Renderer2D*  pR2D;

    virtual void GetRenderInterfaces(Render::Interfaces* p)
    {
        p->pHAL            = pHAL;
        p->pRenderer2D     = pR2D;
        p->pTextureManager = pHAL->GetTextureManager();
        p->RenderThreadID  = 0;
    }
    virtual void PushThreadCommand(Render::ThreadCommand* command)
    {
        if (command) command->Execute();
    }
};

// ---------------------------------------------------------------------------
// Capture screenshot: reads swapchain image to a PPM file.
// Image must be in PRESENT_SRC_KHR layout. Stalls GPU.
// ---------------------------------------------------------------------------

static void CaptureScreenshot(const char* filename, uint32_t imageIdx)
{
    if (!vkDevice || !vkPhysDevice || !vkQueue || !vkCmdPool) return;

    vkDeviceWaitIdle(vkDevice);

    uint32_t w = vkSwapExtent.width;
    uint32_t h = vkSwapExtent.height;
    VkDeviceSize bufSize = (VkDeviceSize)w * h * 4;

    VkBuffer       stagingBuf = VK_NULL_HANDLE;
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
            fputc(px[2], ppm); // R
            fputc(px[1], ppm); // G
            fputc(px[0], ppm); // B
        }
        fclose(ppm);
        printf("Captured frame -> %s (%ux%u)\n", filename, w, h);
    }
    else
    {
        fprintf(stderr, "CaptureVulkan: Failed to open PPM for writing: %s\n", filename);
    }
    vkUnmapMemory(vkDevice, stagingMem);
    vkDestroyBuffer(vkDevice, stagingBuf, nullptr);
    vkFreeMemory(vkDevice, stagingMem, nullptr);
}

// ---------------------------------------------------------------------------
// Vulkan init / cleanup
// ---------------------------------------------------------------------------

static bool CreateSwapchain()
{
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysDevice, vkSurface, &caps);
    vkSwapExtent.width  = Alg::Clamp(WindowWidth,  caps.minImageExtent.width,  caps.maxImageExtent.width);
    vkSwapExtent.height = Alg::Clamp(WindowHeight, caps.minImageExtent.height, caps.maxImageExtent.height);

    if (vkSwapExtent.width == 0 || vkSwapExtent.height == 0) return false;

    uint32_t imgCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imgCount > caps.maxImageCount) imgCount = caps.maxImageCount;

    VkSwapchainCreateInfoKHR ci = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    ci.surface          = vkSurface;
    ci.minImageCount    = imgCount;
    ci.imageFormat      = vkSwapFormat;
    ci.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    ci.imageExtent      = vkSwapExtent;
    ci.imageArrayLayers = 1;
    // Include TRANSFER_SRC_BIT so we can readback pixels for capture
    ci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ci.preTransform     = caps.currentTransform;
    ci.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // Use IMMEDIATE (no VSync) for fast capture; fall back to FIFO if unavailable.
    VkPresentModeKHR desiredMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    {
        uint32_t modeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysDevice, vkSurface, &modeCount, nullptr);
        std::vector<VkPresentModeKHR> modes(modeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysDevice, vkSurface, &modeCount, modes.data());
        bool found = false;
        for (uint32_t i = 0; i < modeCount; i++)
            if (modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) { found = true; break; }
        if (!found) desiredMode = VK_PRESENT_MODE_FIFO_KHR;
    }
    ci.presentMode      = desiredMode;
    ci.clipped          = VK_TRUE;
    if (vkCreateSwapchainKHR(vkDevice, &ci, nullptr, &vkSwapchain) != VK_SUCCESS) return false;

    vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &vkSwapCount, nullptr);
    if (vkSwapCount > 8) vkSwapCount = 8;
    vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &vkSwapCount, vkSwapImages);

    for (uint32_t i = 0; i < vkSwapCount; i++)
    {
        VkImageViewCreateInfo vCI = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        vCI.image    = vkSwapImages[i];
        vCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vCI.format   = vkSwapFormat;
        vCI.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        vkCreateImageView(vkDevice, &vCI, nullptr, &vkSwapViews[i]);
    }
    return true;
}

static bool CreateDepthResources()
{
    VkFormat candidates[] = { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT };
    vkDepthFormat = VK_FORMAT_UNDEFINED;
    for (auto f : candidates)
    {
        VkFormatProperties p;
        vkGetPhysicalDeviceFormatProperties(vkPhysDevice, f, &p);
        if (p.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        { vkDepthFormat = f; break; }
    }
    if (vkDepthFormat == VK_FORMAT_UNDEFINED) return false;

    VkImageCreateInfo iCI = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    iCI.imageType   = VK_IMAGE_TYPE_2D;
    iCI.format      = vkDepthFormat;
    iCI.extent      = { vkSwapExtent.width, vkSwapExtent.height, 1 };
    iCI.mipLevels   = 1; iCI.arrayLayers = 1; iCI.samples = VK_SAMPLE_COUNT_1_BIT;
    iCI.tiling      = VK_IMAGE_TILING_OPTIMAL;
    iCI.usage       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkCreateImage(vkDevice, &iCI, nullptr, &vkDepthImage);

    VkMemoryRequirements mr;
    vkGetImageMemoryRequirements(vkDevice, vkDepthImage, &mr);
    VkMemoryAllocateInfo aI = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    aI.allocationSize  = mr.size;
    aI.memoryTypeIndex = Vulkan::FindMemoryType(vkPhysDevice, mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkAllocateMemory(vkDevice, &aI, nullptr, &vkDepthMemory);
    vkBindImageMemory(vkDevice, vkDepthImage, vkDepthMemory, 0);

    VkImageViewCreateInfo vCI = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    vCI.image    = vkDepthImage;
    vCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vCI.format   = vkDepthFormat;
    vCI.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
    return vkCreateImageView(vkDevice, &vCI, nullptr, &vkDepthView) == VK_SUCCESS;
}

static bool CreateRenderPass()
{
    VkAttachmentDescription atts[2] = {};
    atts[0].format        = vkSwapFormat;
    atts[0].samples       = VK_SAMPLE_COUNT_1_BIT;
    atts[0].loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
    atts[0].storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
    atts[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    atts[0].finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    atts[1].format              = vkDepthFormat;
    atts[1].samples             = VK_SAMPLE_COUNT_1_BIT;
    atts[1].loadOp              = VK_ATTACHMENT_LOAD_OP_CLEAR;
    atts[1].storeOp             = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[1].stencilLoadOp       = VK_ATTACHMENT_LOAD_OP_CLEAR;
    atts[1].stencilStoreOp      = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[1].initialLayout       = VK_IMAGE_LAYOUT_UNDEFINED;
    atts[1].finalLayout         = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference cRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference dRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
    VkSubpassDescription  sp   = {};
    sp.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sp.colorAttachmentCount    = 1;
    sp.pColorAttachments       = &cRef;
    sp.pDepthStencilAttachment = &dRef;

    VkSubpassDependency dep = {};
    dep.srcSubpass    = VK_SUBPASS_EXTERNAL; dep.dstSubpass = 0;
    dep.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dep.dstStageMask  = dep.srcStageMask;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rpCI = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    rpCI.attachmentCount = 2; rpCI.pAttachments  = atts;
    rpCI.subpassCount    = 1; rpCI.pSubpasses     = &sp;
    rpCI.dependencyCount = 1; rpCI.pDependencies  = &dep;
    if (vkCreateRenderPass(vkDevice, &rpCI, nullptr, &vkRenderPass) != VK_SUCCESS) return false;

    // Resume pass: LOAD_OP_LOAD so post-filter restart doesn't discard rendered content
    atts[0].loadOp        = VK_ATTACHMENT_LOAD_OP_LOAD;
    atts[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    atts[1].loadOp        = VK_ATTACHMENT_LOAD_OP_LOAD;
    atts[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    atts[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    return vkCreateRenderPass(vkDevice, &rpCI, nullptr, &vkResumeRenderPass) == VK_SUCCESS;
}

static bool CreateFramebuffers()
{
    for (uint32_t i = 0; i < vkSwapCount; i++)
    {
        VkImageView att[] = { vkSwapViews[i], vkDepthView };
        VkFramebufferCreateInfo fbCI = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        fbCI.renderPass      = vkRenderPass;
        fbCI.attachmentCount = 2;
        fbCI.pAttachments    = att;
        fbCI.width           = vkSwapExtent.width;
        fbCI.height          = vkSwapExtent.height;
        fbCI.layers          = 1;
        if (vkCreateFramebuffer(vkDevice, &fbCI, nullptr, &vkSwapFBs[i]) != VK_SUCCESS) return false;
    }
    return true;
}

static void CleanupSwapchain()
{
    if (vkDepthView)   { vkDestroyImageView(vkDevice, vkDepthView, nullptr);  vkDepthView   = VK_NULL_HANDLE; }
    if (vkDepthImage)  { vkDestroyImage(vkDevice, vkDepthImage, nullptr);     vkDepthImage  = VK_NULL_HANDLE; }
    if (vkDepthMemory) { vkFreeMemory(vkDevice, vkDepthMemory, nullptr);      vkDepthMemory = VK_NULL_HANDLE; }
    for (uint32_t i = 0; i < vkSwapCount; i++)
    {
        if (vkSwapFBs[i])   { vkDestroyFramebuffer(vkDevice, vkSwapFBs[i], nullptr);   vkSwapFBs[i]   = VK_NULL_HANDLE; }
        if (vkSwapViews[i]) { vkDestroyImageView(vkDevice, vkSwapViews[i], nullptr);   vkSwapViews[i] = VK_NULL_HANDLE; }
    }
    if (vkSwapchain) { vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr); vkSwapchain = VK_NULL_HANDLE; }
}

static void CleanupVulkan()
{
    if (vkDevice) vkDeviceWaitIdle(vkDevice);
    for (unsigned i = 0; i < MaxFramesInFlight; i++)
    {
        if (vkImageAvailable[i]) vkDestroySemaphore(vkDevice, vkImageAvailable[i], nullptr);
        if (vkRenderFinished[i]) vkDestroySemaphore(vkDevice, vkRenderFinished[i], nullptr);
        if (vkInFlightFences[i]) vkDestroyFence(vkDevice, vkInFlightFences[i], nullptr);
    }
    if (vkCmdPool)          vkDestroyCommandPool(vkDevice, vkCmdPool, nullptr);
    if (vkResumeRenderPass) { vkDestroyRenderPass(vkDevice, vkResumeRenderPass, nullptr); vkResumeRenderPass = VK_NULL_HANDLE; }
    if (vkRenderPass)       vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
    CleanupSwapchain();
    if (vkSurface)  vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
    if (vkDevice)   vkDestroyDevice(vkDevice, nullptr);
    if (vkInstance) vkDestroyInstance(vkInstance, nullptr);
}

static bool InitVulkan()
{
    VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = "CaptureVulkan";
    appInfo.apiVersion       = VK_API_VERSION_1_0;

    const char* exts[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    };
    VkInstanceCreateInfo instCI = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instCI.pApplicationInfo        = &appInfo;
    instCI.enabledExtensionCount   = 2;
    instCI.ppEnabledExtensionNames = exts;
    VkResult r = vkCreateInstance(&instCI, nullptr, &vkInstance);
    if (r != VK_SUCCESS) { fprintf(stderr, "  vkCreateInstance failed: %d\n", r); return false; }

    // Physical device: prefer discrete GPU
    uint32_t devCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &devCount, nullptr);
    if (!devCount) { fprintf(stderr, "  No physical devices found\n"); return false; }
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

    // Queue family: graphics
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

    float prio = 1.0f;
    VkDeviceQueueCreateInfo qCI = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    qCI.queueFamilyIndex = vkQueueFamily;
    qCI.queueCount       = 1;
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
    r = vkCreateDevice(vkPhysDevice, &dCI, nullptr, &vkDevice);
    if (r != VK_SUCCESS) { fprintf(stderr, "  vkCreateDevice failed: %d\n", r); return false; }
    vkGetDeviceQueue(vkDevice, vkQueueFamily, 0, &vkQueue);

    // Surface
    VkWin32SurfaceCreateInfoKHR surfCI = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    surfCI.hinstance = GetModuleHandle(NULL);
    surfCI.hwnd      = hWnd;
    r = vkCreateWin32SurfaceKHR(vkInstance, &surfCI, nullptr, &vkSurface);
    if (r != VK_SUCCESS) { fprintf(stderr, "  vkCreateWin32SurfaceKHR failed: %d\n", r); return false; }

    VkBool32 presentSupport = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysDevice, vkQueueFamily, vkSurface, &presentSupport);
    if (!presentSupport) { fprintf(stderr, "  Present not supported\n"); return false; }

    // Pick surface format
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

    // Command pool + buffers
    VkCommandPoolCreateInfo cpCI = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    cpCI.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cpCI.queueFamilyIndex = vkQueueFamily;
    if (vkCreateCommandPool(vkDevice, &cpCI, nullptr, &vkCmdPool) != VK_SUCCESS) return false;

    VkCommandBufferAllocateInfo cbAI = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cbAI.commandPool        = vkCmdPool;
    cbAI.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbAI.commandBufferCount = MaxFramesInFlight;
    vkAllocateCommandBuffers(vkDevice, &cbAI, vkCmdBuffers);

    // Sync objects
    VkSemaphoreCreateInfo semCI = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo     fenCI = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (unsigned i = 0; i < MaxFramesInFlight; i++)
    {
        vkCreateSemaphore(vkDevice, &semCI, nullptr, &vkImageAvailable[i]);
        vkCreateSemaphore(vkDevice, &semCI, nullptr, &vkRenderFinished[i]);
        vkCreateFence(vkDevice, &fenCI, nullptr, &vkInFlightFences[i]);
    }

    if (!CreateSwapchain())      { fprintf(stderr, "  CreateSwapchain failed\n"); return false; }
    if (!CreateDepthResources()) { fprintf(stderr, "  CreateDepthResources failed\n"); return false; }
    if (!CreateRenderPass())     { fprintf(stderr, "  CreateRenderPass failed\n"); return false; }
    if (!CreateFramebuffers())   { fprintf(stderr, "  CreateFramebuffers failed\n"); return false; }

    return true;
}

// ---------------------------------------------------------------------------
// Hidden window proc (no input handling needed for capture)
// ---------------------------------------------------------------------------

static LRESULT CALLBACK CaptureWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_DESTROY || msg == WM_CLOSE)
        PostQuitMessage(0);
    return DefWindowProcA(hwnd, msg, wp, lp);
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: CaptureVulkan.exe <swf_path> <output_dir> --frames 1,10,30,60 [--width 1024] [--height 768]\n");
        return 1;
    }

    const char*      swfPath   = argv[1];
    const char*      outputDir = argv[2];
    std::vector<int> frames;
    int              width  = 1024;
    int              height = 768;

    for (int i = 3; i < argc; i++)
    {
        if (_stricmp(argv[i], "--frames") == 0 && i + 1 < argc)
        {
            ++i;
            char buf[256];
            strncpy_s(buf, argv[i], 255);
            char* tok = strtok(buf, ",");
            while (tok) { frames.push_back(atoi(tok)); tok = strtok(NULL, ","); }
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
    std::sort(frames.begin(), frames.end());
    int maxFrame = frames.back();

    WindowWidth  = (unsigned)width;
    WindowHeight = (unsigned)height;

    // Create output directory
    CreateDirectoryA(outputDir, NULL);

    // GFx system
    SF::SysAllocMalloc sysAlloc;
    SF::GFx::System gfxInit(&sysAlloc);

    // Hidden window (DXGI/Vulkan surface requires an HWND on Windows)
    HINSTANCE hInst = GetModuleHandle(NULL);
    WNDCLASSEXA wc  = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = CaptureWndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = "CaptureVulkan_Class";
    RegisterClassExA(&wc);

    hWnd = CreateWindowExA(0, "CaptureVulkan_Class", "CaptureVulkan",
        WS_POPUP,
        0, 0, width, height,
        NULL, NULL, hInst, NULL);
    if (!hWnd) { fprintf(stderr, "CaptureVulkan: Failed to create window\n"); return 1; }
    ShowWindow(hWnd, SW_HIDE);

    if (!InitVulkan())
    {
        fprintf(stderr, "CaptureVulkan: Vulkan init failed\n");
        CleanupVulkan();
        return 1;
    }

    // Init Scaleform HAL
    pHAL = *SF_NEW Vulkan::HAL();
    Vulkan::HALInitParams halParams(
        vkDevice, vkPhysDevice, vkQueue, vkQueueFamily,
        vkCmdPool, vkCmdBuffers[0], vkRenderPass, vkSwapFormat, vkInstance);
    if (!pHAL->InitHAL(halParams))
    {
        fprintf(stderr, "CaptureVulkan: HAL init failed\n");
        pHAL.Clear();
        CleanupVulkan();
        return 1;
    }

    pRenderer = *SF_NEW Renderer2D(pHAL.GetPtr());

    CaptureThreadCommandQueue* queue = new CaptureThreadCommandQueue();
    queue->pHAL = pHAL.GetPtr();
    queue->pR2D = pRenderer.GetPtr();

    // Load SWF
    Loader* pLoader = new Loader();
    pLoader->SetLog(Ptr<Scaleform::Log>(*SF_NEW CaptureLog()));

    Ptr<FileOpener> pFileOpener = *new FileOpener;
    pLoader->SetFileOpener(pFileOpener);

    Ptr<FontProviderWin32> fontProvider = *new FontProviderWin32(::GetDC(0));
    pLoader->SetFontProvider(fontProvider);

    Ptr<ASSupport> pAS2Support = *new GFx::AS2Support();
    pLoader->SetAS2Support(pAS2Support);
    Ptr<ASSupport> pAS3Support = *new GFx::AS3Support();
    pLoader->SetAS3Support(pAS3Support);

    Ptr<GFx::ImageCreator> pImageCreator = *SF_NEW GFx::ImageCreator(pHAL->GetTextureManager());
    pLoader->SetImageCreator(pImageCreator);

    // Register image file handlers (JPEG, PNG, etc.) for SWF-embedded bitmaps
    Ptr<GFx::ImageFileHandlerRegistry> pImgReg = *new GFx::ImageFileHandlerRegistry();
#ifdef SF_ENABLE_LIBJPEG
    pImgReg->AddHandler(&SF::Render::JPEG::FileReader::Instance);
#endif
#ifdef SF_ENABLE_LIBPNG
    pImgReg->AddHandler(&SF::Render::PNG::FileReader::Instance);
#endif
    pImgReg->AddHandler(&SF::Render::TGA::FileReader::Instance);
    pImgReg->AddHandler(&SF::Render::DDS::FileReader::Instance);
    pLoader->SetImageFileHandlerRegistry(pImgReg);

    Ptr<GFx::ActionControl> pActCtrl = *SF_NEW GFx::ActionControl(GFx::ActionControl::Action_ErrorSuppress);
    pLoader->SetActionControl(pActCtrl);

    Ptr<MovieDef> pMovieDef = *pLoader->CreateMovie(swfPath, Loader::LoadAll);
    if (!pMovieDef)
    {
        fprintf(stderr, "CaptureVulkan: Failed to load SWF: %s\n", swfPath);
        return 1;
    }

    Ptr<Movie> pMovie = *pMovieDef->CreateInstance(false, 0, 0, queue);
    if (!pMovie)
    {
        fprintf(stderr, "CaptureVulkan: Failed to create movie instance\n");
        return 1;
    }

    pMovie->SetViewport((int)WindowWidth, (int)WindowHeight, 0, 0, (int)WindowWidth, (int)WindowHeight);
    pMovie->SetBackgroundAlpha(0.0f);
    pMovie->SetMouseCursorCount(1);

    // Main capture loop
    int      frameNumber = 0; // 1-based
    int      captureIdx  = 0;
    bool     running     = true;
    MSG      msg         = {};

    while (running && captureIdx < (int)frames.size())
    {
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) running = false;
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (!running) break;

        frameNumber++;

        unsigned frameIdx = CurrentFrame % MaxFramesInFlight;
        vkWaitForFences(vkDevice, 1, &vkInFlightFences[frameIdx], VK_TRUE, UINT64_MAX);

        VkResult acquireResult = vkAcquireNextImageKHR(vkDevice, vkSwapchain, UINT64_MAX,
            vkImageAvailable[frameIdx], VK_NULL_HANDLE, &CurrentImage);
        if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
        {
            fprintf(stderr, "CaptureVulkan: vkAcquireNextImageKHR failed at frame %d\n", frameNumber);
            break;
        }
        vkResetFences(vkDevice, 1, &vkInFlightFences[frameIdx]);

        VkCommandBuffer cmd = vkCmdBuffers[frameIdx];
        vkResetCommandBuffer(cmd, 0);
        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        vkBeginCommandBuffer(cmd, &beginInfo);

        // Black clear color for consistent background
        VkClearValue clears[2] = {};
        clears[0].color        = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clears[1].depthStencil = { 0.0f, 0 };

        VkRenderPassBeginInfo rpBI = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        rpBI.renderPass        = vkRenderPass;
        rpBI.framebuffer       = vkSwapFBs[CurrentImage];
        rpBI.renderArea.extent = vkSwapExtent;
        rpBI.clearValueCount   = 2;
        rpBI.pClearValues      = clears;
        vkCmdBeginRenderPass(cmd, &rpBI, VK_SUBPASS_CONTENTS_INLINE);

        // Render Scaleform
        pHAL->SetCommandBuffer(cmd);
        pHAL->InRenderPass = true;
        pHAL->SetMainRenderTarget(vkRenderPass, vkResumeRenderPass,
                                  vkSwapFBs[CurrentImage], vkSwapExtent);
        Vulkan::RenderEvent::UpdateCommandBuffer(cmd);

        pMovie->Advance(1.0f / 30.0f, 0);

        pRenderer->BeginFrame();

        VkFence rsyncFence = VK_NULL_HANDLE;
        MovieDisplayHandle hmd = pMovie->GetDisplayHandle();
        if (hmd.NextCapture(pRenderer->GetContextNotify()))
            pRenderer->Display(hmd);

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
        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = &vkImageAvailable[frameIdx];
        submitInfo.pWaitDstStageMask    = &waitStage;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &cmd;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = &vkRenderFinished[frameIdx];
        vkQueueSubmit(vkQueue, 1, &submitInfo, vkInFlightFences[frameIdx]);

        if (rsyncFence != VK_NULL_HANDLE)
        {
            VkSubmitInfo emptySubmit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
            vkQueueSubmit(vkQueue, 1, &emptySubmit, rsyncFence);
        }

        VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = &vkRenderFinished[frameIdx];
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = &vkSwapchain;
        presentInfo.pImageIndices      = &CurrentImage;
        vkQueuePresentKHR(vkQueue, &presentInfo);

        // Capture this frame if it's in our list (after present so image is in PRESENT_SRC layout)
        if (captureIdx < (int)frames.size() && frameNumber == frames[captureIdx])
        {
            char outPath[MAX_PATH];
            _snprintf_s(outPath, MAX_PATH, _TRUNCATE, "%s\\frame_%04d.ppm",
                outputDir, frameNumber);
            CaptureScreenshot(outPath, CurrentImage);
            captureIdx++;
        }

        CurrentFrame++;

        if (frameNumber >= maxFrame)
            running = false;
    }

    // Cleanup
    if (vkDevice) vkDeviceWaitIdle(vkDevice);
    pMovie.Clear();
    pMovieDef.Clear();
    delete pLoader;
    pLoader = nullptr;
    delete queue;
    pRenderer.Clear();
    if (pHAL) { pHAL->ShutdownHAL(); pHAL.Clear(); }
    CleanupVulkan();

    DestroyWindow(hWnd);
    printf("CaptureVulkan: Done.\n");
    return 0;
}
