/**************************************************************************

Filename    :   Vulkan_Sync.h
Content     :   Vulkan fence-based GPU/CPU synchronization.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#ifndef INC_SF_Vulkan_Sync_H
#define INC_SF_Vulkan_Sync_H

#include "Render/Render_Sync.h"
#include "Render/Vulkan/Vulkan_Config.h"
#include "Kernel/SF_Threads.h"

namespace Scaleform { namespace Render { namespace Vulkan {

class RenderSync : public Render::RenderSync
{
public:
    RenderSync() : pDevice(VK_NULL_HANDLE), pQueue(VK_NULL_HANDLE), pNextEndFrameFence(VK_NULL_HANDLE), LastReleasedFence(VK_NULL_HANDLE) {}

    bool SetDevice(VkDevice device, VkQueue queue)
    {
        if (device == VK_NULL_HANDLE)
        {
            // Release outstanding frames BEFORE clearing pDevice, so that
            // ReleaseFence can still call vkDestroyFence on each VkFence.
            ReleaseOutstandingFrames();

            // Destroy any in-progress fence that hasn't been submitted yet.
            if (pNextEndFrameFence != VK_NULL_HANDLE && pDevice != VK_NULL_HANDLE)
            {
                vkDestroyFence(pDevice, pNextEndFrameFence, nullptr);
                pNextEndFrameFence = VK_NULL_HANDLE;
            }

            pDevice = VK_NULL_HANDLE;
            pQueue  = VK_NULL_HANDLE;
            return true;
        }

        VkFenceCreateInfo ci = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        VkFence testFence;
        VkResult r = vkCreateFence(device, &ci, nullptr, &testFence);
        if (r == VK_SUCCESS)
        {
            vkDestroyFence(device, testFence, nullptr);
            pDevice = device;
            pQueue  = queue;
        }
        return r == VK_SUCCESS;
    }

    virtual void BeginFrame()
    {
        if (!pDevice) return;

        // If a previous fence was never consumed by EndFrame, destroy it now
        // to avoid leaking VkFence handles.
        if (pNextEndFrameFence != VK_NULL_HANDLE)
        {
            vkDestroyFence(pDevice, pNextEndFrameFence, nullptr);
            pNextEndFrameFence = VK_NULL_HANDLE;
        }

        VkFenceCreateInfo ci = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        VkResult r = vkCreateFence(pDevice, &ci, nullptr, &pNextEndFrameFence);
        if (r != VK_SUCCESS)
            pNextEndFrameFence = VK_NULL_HANDLE;

        Render::RenderSync::BeginFrame();
    }

    virtual bool EndFrame()
    {
        if (!pDevice) return true;
        if (!Render::RenderSync::EndFrame())
            return false;
        // IMPORTANT CONTRACT: The platform layer MUST call vkQueueSubmit() with the
        // fence returned by GetNextEndFrameFence() before the next BeginFrame().
        // If the fence is never submitted, IsPending() will report it as pending
        // forever, which will block the mesh cache and eventually stall rendering.
        SF_ASSERT(pNextEndFrameFence != VK_NULL_HANDLE || !pDevice);
        pNextEndFrameFence = VK_NULL_HANDLE;
        return true;
    }

    VkFence GetNextEndFrameFence() const { return pNextEndFrameFence; }

    virtual void KickOffFences(FenceType waitType)
    {
        SF_UNUSED(waitType);
    }

protected:
    virtual UInt64 SetFence()
    {
        return (UInt64)pNextEndFrameFence;
    }

    virtual bool IsPending(FenceType waitType, UInt64 handle, const FenceFrame& parent)
    {
        SF_UNUSED2(waitType, parent);
        VkFence fence = (VkFence)handle;
        if (fence == VK_NULL_HANDLE) return false;
        if (FenceFrames.IsEmpty()) return false;
        if (fence == pNextEndFrameFence) return true;
        return vkGetFenceStatus(pDevice, fence) != VK_SUCCESS;
    }

    virtual void WaitFence(FenceType waitType, UInt64 handle, const FenceFrame& parent)
    {
        SF_UNUSED2(waitType, parent);
        VkFence fence = (VkFence)handle;
        if (fence == VK_NULL_HANDLE) return;
        vkWaitForFences(pDevice, 1, &fence, VK_TRUE, UINT64_MAX);
    }

    virtual void ReleaseFence(UInt64 handle)
    {
        VkFence fence = (VkFence)handle;
        if (fence == VK_NULL_HANDLE || !pDevice) return;
        if (fence == LastReleasedFence) return;
        vkDestroyFence(pDevice, fence, nullptr);
        LastReleasedFence = fence;
    }

private:
    VkDevice pDevice;
    VkQueue  pQueue;
    VkFence  pNextEndFrameFence;
    VkFence  LastReleasedFence;
};

}}} // Scaleform::Render::Vulkan

#endif // INC_SF_Vulkan_Sync_H
