/**********************************************************************

PublicHeader:   Render
Filename    :   PS4_Sync.cpp
Content     :   
Created     :   2013/12/20
Authors     :   Bart Muzzin

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render/PS4/PS4_Sync.h"
#include "Render/PS4/PS4_HAL.h"

namespace Scaleform { namespace Render { namespace PS4 {

Scaleform::Render::PS4::RenderSync::RenderSync( PS4::HAL* hal ) : Render::RenderSync(), pHAL(hal), GnmxCtx(0), CurFence(0), pLabelAddress(0)
{

}

void Scaleform::Render::PS4::RenderSync::SetContext( sceGnmxContextType* context, Render::MemoryManager& memoryManager )
{
    if (context && !pLabelAddress)
    {
        // If we are setting the context, and we don't have a wait label address, allocate one now.
        void *labelAddress = memoryManager.Alloc(sizeof(UInt64), sizeof(uint64_t), Memory_Orbis_UC_GARLIC_NONVOLATILE);
        pLabelAddress = reinterpret_cast<UInt64*>(labelAddress);
    }
    else if (!context && pLabelAddress)
    {
        // If we are setting the context to NULL, it means that we should free the label, if it was allocated.
        memoryManager.Free(pLabelAddress, sizeof(UInt64));
        pLabelAddress = 0;
    }

    GnmxCtx = context;

    // NOTE: Do not set CurFence to zero. In the use-case where the sce::Gnm::DrawCommandBuffer is switched per-frame, this
    // can cause GPU resources that are in use to be erroneously evicted.
    // CurFence = 0;
}

void Scaleform::Render::PS4::RenderSync::KickOffFences( FenceType waitType )
{
    SF_UNUSED(waitType);
    pHAL->SubmitAndResetStates();
}

Scaleform::UInt64 Scaleform::Render::PS4::RenderSync::SetFence()
{
    SF_DEBUG_ASSERT(GnmxCtx != 0, "Must call SetContext before calling SetFence.");
    ++CurFence;
    GnmxCtx->writeAtEndOfPipe(
        sce::Gnm::EndOfPipeEventType::kEopFlushCbDbCaches, 
        sce::Gnm::EventWriteDest::kEventWriteDestMemory,
        pLabelAddress, 
        sce::Gnm::EventWriteSource::kEventWriteSource64BitsImmediate,
        CurFence, 
        sce::Gnm::CacheAction::kCacheActionNone,
        sce::Gnm::CachePolicy::kCachePolicyLru);

    return CurFence;
}

bool Scaleform::Render::PS4::RenderSync::IsPending( FenceType waitType, UInt64 handle, const FenceFrame& parent )
{
    SF_DEBUG_ASSERT(GnmxCtx != 0, "Must call SetContext before calling SetFence.");
    return handle > *pLabelAddress;
}

void Scaleform::Render::PS4::RenderSync::WaitFence( FenceType waitType, UInt64 handle, const FenceFrame& parent )
{
    // Wait for the label to be written.
    unsigned totalWaitTime = 0;
    const unsigned MaxWaitTime = 10 * 1000; // 100ms maximum wait.
    bool first = true;
    while (IsPending(waitType, handle, parent))
    {
        // When we reach a pending fence, do a submit, otherwise the GPU might not process the commands to reach the fence.
        if (first)
        {
            KickOffFences(waitType);
            first = false;
        }

        usleep(10);        
        totalWaitTime += 10;
        SF_DEBUG_ASSERT(totalWaitTime < MaxWaitTime, "Failing waiting on writeEndOfPipeEvent value.");

        // In shipping builds, do not freeze.
#if defined(SF_BUILD_SHIPPING)
        if ( totalWaitTime > MaxWaitTime)
            break;
#endif
    }
}

}}}; // Scaleform::Render::PS4
