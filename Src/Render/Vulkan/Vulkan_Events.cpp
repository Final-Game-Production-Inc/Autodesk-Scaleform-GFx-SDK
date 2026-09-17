/**************************************************************************

Filename    :   Vulkan_Events.cpp
Content     :   Vulkan GPU debug event statics.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#include "Render/Vulkan/Vulkan_Events.h"

namespace Scaleform { namespace Render { namespace Vulkan {

PFN_vkCmdBeginDebugUtilsLabelEXT RenderEvent::pfnBegin = nullptr;
PFN_vkCmdEndDebugUtilsLabelEXT   RenderEvent::pfnEnd   = nullptr;
VkCommandBuffer                   RenderEvent::pCmdBuf  = VK_NULL_HANDLE;

}}} // Scaleform::Render::Vulkan
