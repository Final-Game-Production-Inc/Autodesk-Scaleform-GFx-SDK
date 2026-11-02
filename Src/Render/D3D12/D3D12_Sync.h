/**************************************************************************

    PublicHeader:   Render
    Filename    :   D3D12_Sync.h
    Content     :   GPU/CPU synchronization (fencing) for the D3D12 renderer.
                    Synchronization is implemented with a single ID3D12Fence
                    object: SetFence signals the fence with a monotonically
                    increasing value, IsPending/WaitFence compare/block on the
                    fence completion value.
    Created     :   2026/10/20
    Authors     :   Final Game Production Inc.

    Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                         Copyright 2026 Final Game Production Inc. All Rights Reserved.

    Use of this software is subject to the terms of the Autodesk license
    agreement provided at the time of installation or download, or which
    otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_D3D12_SYNC_H
#define INC_SF_D3D12_SYNC_H
#pragma once

#include "Render/D3D12/D3D12_Config.h"
#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_ListAlloc.h"

#include "Render/Render_Sync.h"

namespace Scaleform { namespace Render { namespace D3D12 {

// FenceWrapper for the D3D12 sync implementation. The D3D12 fence is a single
// shared object, so the wrapper does not need to carry per-wrapper GPU state;
// it exists to satisfy the FenceWrapperList pooling interface.
class FenceWrapper : public Render::FenceWrapper
{
public:
    FenceWrapper() : Render::FenceWrapper() { }
};

// D3D12 RenderSync implementation, based on a single ID3D12Fence.
// A fence value is recorded every time a fence is set; the GPU is considered
// "caught up" to a fence value when ID3D12Fence::GetCompletedValue returns a
// value >= the recorded one.
class RenderSync : public Render::RenderSync, public FenceWrapperList
{
public:
    RenderSync();
    virtual ~RenderSync();

    // Initializes the sync with the given device and command queue.
    bool Init(ID3D12Device* pdevice, ID3D12CommandQueue* pqueue);

    // Allows the sync to be (re)initialized with a different device/queue.
    void SetDevice(ID3D12Device* pdevice, ID3D12CommandQueue* pqueue);

    // Signals that a new frame has begun.
    virtual void BeginFrame();

    // Signals that the frame has ended. Returns true if the frame completed.
    virtual bool EndFrame();

    // Kicks off any outstanding fences. On D3D12 all fences are signaled
    // directly on the single command queue, so there is nothing to flush.
    virtual void KickOffFences(FenceType waitType) { SF_UNUSED(waitType); }

    // Signals every fence value recorded by SetFence() since the last flush.
    // Recording a fence must not signal it immediately: the drawing commands are
    // still being recorded into a command list at that point, and the list is
    // only executed later (platform Present). Signaling early made
    // IsPending() report "complete" for work that had not even been submitted,
    // so the mesh cache freed vertex/index buffers that the command list still
    // referenced (D3D12 debug layer: "resource ... was deleted prior to closing
    // the command list") and the GPU hung. The platform calls this right after
    // ID3D12CommandQueue::ExecuteCommandLists.
    void FlushPendingSignals();

protected:

    // Allocates a pooled fence wrapper.
    virtual Render::FenceWrapper* allocateWrapper();

    // Determines whether the fence with the given handle is still pending.
    virtual bool IsPending(FenceType waitType, UInt64 handle, const FenceFrame& parent);

    // Waits for the fence with the given handle to complete on the CPU.
    virtual void WaitFence(FenceType waitType, UInt64 handle, const FenceFrame& parent);

    // Signals a new fence on the command queue and returns its handle (value).
    virtual UInt64 SetFence();

    // Releases a fence handle. Not used; the D3D12 fence object is shared.
    virtual void ReleaseFence(UInt64 apiHandle) { SF_UNUSED(apiHandle); }

    ID3D12Device*        pDevice;
    ID3D12CommandQueue*  pCommandQueue;
    ID3D12Fence*         pFence;

    // The value that will be assigned to the next fence.
    UInt64               NextFenceValue;

    // Fence values handed out by SetFence() that have not been signaled on the
    // command queue yet (signaled by FlushPendingSignals after submission).
    Array<UInt64>        PendingSignals;

    // Handle to the event object used for CPU waits.
    HANDLE               WaitEvent;
};

}}} // Scaleform::Render::D3D12

#endif // INC_SF_D3D12_SYNC_H
