/**************************************************************************

Filename    :   Vulkan_Sync.h
Content     :   Vulkan fence-based GPU/CPU synchronization.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#ifndef INC_SF_Vulkan_Sync_H
#define INC_SF_Vulkan_Sync_H

#include <stdlib.h> // TEMP GFX_VKSYNC
#include <stdio.h>

#include "Render/Render_Sync.h"
#include "Render/Vulkan/Vulkan_Config.h"
#include "Render/Render_DebugEnv.h"
#include "Kernel/SF_Threads.h"

namespace Scaleform { namespace Render { namespace Vulkan {

class RenderSync : public Render::RenderSync
{
public:
    RenderSync() : pDevice(VK_NULL_HANDLE), pQueue(VK_NULL_HANDLE),
                   ExternalFence(VK_NULL_HANDLE), LastReleasedFence(VK_NULL_HANDLE), BeginCount(0), EndCount(0), FenceCounter(0) {}

    bool SetDevice(VkDevice device, VkQueue queue)
    {
        if (device == VK_NULL_HANDLE)
        {
            ReleaseOutstandingFrames();
            pDevice = VK_NULL_HANDLE;
            pQueue  = VK_NULL_HANDLE;
            ExternalFence = VK_NULL_HANDLE;
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

    // Called by the platform layer at the start of each frame (beginFrame),
    // BEFORE HAL::BeginFrame() runs. Supplies the VkFence that vkQueueSubmit
    // will signal when the GPU finishes the frame. RenderSync uses this same
    // fence for IsPending() queries, so GPU completion is reflected correctly.
    //
    // Lifecycle contract:
    //   - Platform layer owns the VkFence (e.g. vkInFlightFences[frame]).
    //   - Platform layer must vkResetFences() the fence BEFORE vkQueueSubmit,
    //     and only after waiting on it from the previous frame.
    //   - We never vkDestroyFence() the external fence; ReleaseFence() skips it.
    void SetExternalFence(VkFence fence) { ExternalFence = fence; }
    VkFence GetExternalFence() const     { return ExternalFence; }

    unsigned BeginCount, EndCount;

    virtual void BeginFrame()
    {
        if (!pDevice) return;
        // Use the platform-supplied fence as the end-of-frame fence.
        // We do NOT create our own VkFence — the platform layer already
        // creates one per frame slot and submits it via vkQueueSubmit.
        ++BeginCount;
        Render::RenderSync::BeginFrame();
    }

    virtual bool EndFrame()
    {
        if (!pDevice) return true;
        ++EndCount;
        static const bool sSyncTrace = IsDebugEnvSet("GFX_VKSYNC");
        if (sSyncTrace)
        {
            VkResult st = ExternalFence ? vkGetFenceStatus(pDevice, ExternalFence) : VK_NOT_READY;
            fprintf(stderr, "[VKSYNC] begin=%u end=%u fence=%p stat=%d\n", BeginCount, EndCount, (void*)ExternalFence, (int)st);
            fflush(stderr);
        }
        return Render::RenderSync::EndFrame();
    }

    virtual void KickOffFences(FenceType waitType)
    {
        SF_UNUSED(waitType);
    }

protected:
    // The renderer core records resources under the value returned here and only
    // reclaims them once IsPending(thatValue) turns false. Returning the raw VkFence
    // was wrong: only MaxFramesInFlight handles exist and they repeat every few
    // frames, so resources from an older frame looked "complete" the moment the
    // slot was reused - which freed StateBag/FilterSet data while the tree still
    // referenced it. Emit a monotonically increasing value instead (D3D12 does
    // the same) and remember which fence signals it.
    virtual UInt64 SetFence()
    {
        UInt64 value = ++FenceCounter;
        FenceTrackers[value % FenceTrackerCount].Value = value;
        FenceTrackers[value % FenceTrackerCount].Fence = ExternalFence;
        return value;
    }

    VkFence FindFence(UInt64 value) const
    {
        const FenceTracker& t = FenceTrackers[value % FenceTrackerCount];
        return (t.Value == value) ? t.Fence : VK_NULL_HANDLE;
    }
    virtual bool IsPending(FenceType waitType, UInt64 handle, const FenceFrame& parent)
    {
        SF_UNUSED2(waitType, parent);
        VkFence fence = FindFence(handle);
        // No fence means the platform has not supplied one for this frame yet.
        // Treat the resource as still in flight: reporting "not pending" here
        // released glyph/state data that the GPU (and the main thread) still used.
        if (fence == VK_NULL_HANDLE) return true;   // unknown frame: keep it alive
        if (FenceFrames.IsEmpty()) return false;
        return vkGetFenceStatus(pDevice, fence) != VK_SUCCESS;
    }

    virtual void WaitFence(FenceType waitType, UInt64 handle, const FenceFrame& parent)
    {
        SF_UNUSED2(waitType, parent);
        VkFence fence = FindFence(handle);
        if (fence == VK_NULL_HANDLE) return;
        vkWaitForFences(pDevice, 1, &fence, VK_TRUE, UINT64_MAX);
    }

    virtual void ReleaseFence(UInt64 handle)
    {
        // The fence is owned by the platform layer (vkInFlightFences slot).
        // We must NOT vkDestroyFence it here — that would break the next
        // frame's wait/submit cycle.
        SF_UNUSED(handle);
    }

private:
    VkDevice pDevice;
    VkQueue  pQueue;
    VkFence  ExternalFence;
    VkFence  LastReleasedFence;

    struct FenceTracker
    {
        UInt64  Value;
        VkFence Fence;
        FenceTracker() : Value(0), Fence(VK_NULL_HANDLE) {}
    };
    static const unsigned FenceTrackerCount = 128;
    FenceTracker FenceTrackers[FenceTrackerCount];
    UInt64       FenceCounter;
};

}}}; // Scaleform::Render::Vulkan

#endif // INC_SF_Vulkan_Sync_H

