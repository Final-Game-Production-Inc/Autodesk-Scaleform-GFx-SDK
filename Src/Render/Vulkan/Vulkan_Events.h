/**************************************************************************

Filename    :   Vulkan_Events.h
Content     :   Vulkan GPU debug markers via VK_EXT_debug_utils.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#ifndef INC_SF_Vulkan_Events_H
#define INC_SF_Vulkan_Events_H

#include "Kernel/SF_String.h"
#include "Render/Vulkan/Vulkan_Config.h"
#include "Render/Render_Events.h"

namespace Scaleform { namespace Render { namespace Vulkan {

class RenderEvent : public Render::RenderEvent
{
public:
    static void InitializeEvents(VkInstance instance, VkCommandBuffer cmdBuf)
    {
        pCmdBuf = cmdBuf;
        pfnBegin = (PFN_vkCmdBeginDebugUtilsLabelEXT)
            vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
        pfnEnd = (PFN_vkCmdEndDebugUtilsLabelEXT)
            vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
    }

    static void UpdateCommandBuffer(VkCommandBuffer cmdBuf)
    {
        pCmdBuf = cmdBuf;
    }

    static void ShutdownEvents()
    {
        pfnBegin = nullptr;
        pfnEnd   = nullptr;
        pCmdBuf  = VK_NULL_HANDLE;
    }

    virtual void Begin(String eventName)
    {
#if !defined(SF_BUILD_SHIPPING)
        if (!pfnBegin || pCmdBuf == VK_NULL_HANDLE) return;
        VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
        label.pLabelName = eventName.ToCStr();
        label.color[0] = 0.0f;
        label.color[1] = 0.0f;
        label.color[2] = 0.0f;
        label.color[3] = 1.0f;
        pfnBegin(pCmdBuf, &label);
#else
        SF_UNUSED(eventName);
#endif
    }

    virtual void End()
    {
#if !defined(SF_BUILD_SHIPPING)
        if (!pfnEnd || pCmdBuf == VK_NULL_HANDLE) return;
        pfnEnd(pCmdBuf);
#endif
    }

    // NOTE: static state limits the system to a single HAL instance.
    // If multiple HAL instances are needed, these should be moved to
    // per-instance storage or passed via the HAL.
    static PFN_vkCmdBeginDebugUtilsLabelEXT  pfnBegin;
    static PFN_vkCmdEndDebugUtilsLabelEXT    pfnEnd;
    static VkCommandBuffer                    pCmdBuf;
};

}}} // Scaleform::Render::Vulkan

#endif // INC_SF_Vulkan_Events_H
