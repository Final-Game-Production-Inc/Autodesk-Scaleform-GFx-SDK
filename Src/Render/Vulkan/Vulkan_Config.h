/**************************************************************************

Filename    :   Vulkan_Config.h
Content     :   Vulkan backend preprocessor configuration and API includes.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#ifndef INC_SF_VULKAN_CONFIG_H
#define INC_SF_VULKAN_CONFIG_H
#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#ifndef SF_VK_MAX_FRAMES_IN_FLIGHT
#define SF_VK_MAX_FRAMES_IN_FLIGHT 2
#endif

#ifndef SF_VK_UBO_RING_SIZE
#define SF_VK_UBO_RING_SIZE (8 * 1024 * 1024)
#endif

#ifndef SF_VK_DESCRIPTOR_POOL_MAX_SETS
#define SF_VK_DESCRIPTOR_POOL_MAX_SETS 2048
#endif

#ifndef SF_VK_MAX_TEXTURE_STAGES
#define SF_VK_MAX_TEXTURE_STAGES 4
#endif

#ifndef SF_VK_ENABLE_VALIDATION
    #if defined(SF_BUILD_DEBUG)
        #define SF_VK_ENABLE_VALIDATION 1
    #else
        #define SF_VK_ENABLE_VALIDATION 0
    #endif
#endif

namespace Scaleform { namespace Render { namespace Vulkan {

inline const char* VkResultToString(VkResult r)
{
    switch(r)
    {
    case VK_SUCCESS:                        return "VK_SUCCESS";
    case VK_NOT_READY:                      return "VK_NOT_READY";
    case VK_TIMEOUT:                        return "VK_TIMEOUT";
    case VK_ERROR_OUT_OF_HOST_MEMORY:       return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:     return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:    return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:              return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:        return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:        return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:    return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:      return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_SUBOPTIMAL_KHR:                 return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:          return "VK_ERROR_OUT_OF_DATE_KHR";
    default:                                return "VK_UNKNOWN";
    }
}

inline uint32_t FindMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physDevice, &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    return UINT32_MAX;
}

}}} // Scaleform::Render::Vulkan

#define SF_VK_CHECK(expr) \
    do { VkResult _vkr = (expr); \
         if (_vkr != VK_SUCCESS) { SF_DEBUG_WARNING2(1, "Vulkan: %s failed: %s\n", #expr, Scaleform::Render::Vulkan::VkResultToString(_vkr)); } \
    } while(0)

#define SF_VK_CHECK_RETURN(expr, ret) \
    do { VkResult _vkr = (expr); \
         if (_vkr != VK_SUCCESS) { SF_DEBUG_WARNING2(1, "Vulkan: %s failed: %s\n", #expr, Scaleform::Render::Vulkan::VkResultToString(_vkr)); return ret; } \
    } while(0)

#endif // INC_SF_VULKAN_CONFIG_H
