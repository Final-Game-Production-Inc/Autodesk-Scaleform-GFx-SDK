/**************************************************************************

    Filename    :   D3D12_Sync.cpp
    Content     :   D3D12 GPU/CPU synchronization (fencing) implementation.
    Created     :   2026/10/20
    Authors     :   Final Game Production Inc.

    Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                         Copyright 2026 Final Game Production Inc. All Rights Reserved.

    Use of this software is subject to the terms of the Autodesk license
    agreement provided at the time of installation or download, or which
    otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/D3D12/D3D12_Sync.h"
#include "Kernel/SF_Debug.h"

#include <windows.h>

namespace Scaleform { namespace Render { namespace D3D12 {

RenderSync::RenderSync() :
    Render::RenderSync(),
    FenceWrapperList(0),
    pDevice(0), pCommandQueue(0), pFence(0), NextFenceValue(1), WaitEvent(0)
{
}

RenderSync::~RenderSync()
{
    SetDevice(0, 0);
}

Render::FenceWrapper* RenderSync::allocateWrapper()
{
    return SF_NEW FenceWrapper();
}

bool RenderSync::Init(ID3D12Device* pdevice, ID3D12CommandQueue* pqueue)
{
    SetDevice(pdevice, pqueue);
    return pFence != 0;
}

void RenderSync::SetDevice(ID3D12Device* pdevice, ID3D12CommandQueue* pqueue)
{
    if (pFence)
    {
        pFence->Release();
        pFence = 0;
    }
    if (WaitEvent)
    {
        CloseHandle(WaitEvent);
        WaitEvent = 0;
    }
    if (pDevice)
    {
        pDevice->Release();
        pDevice = 0;
    }
    if (pCommandQueue)
    {
        pCommandQueue->Release();
        pCommandQueue = 0;
    }

    pDevice       = pdevice;
    pCommandQueue = pqueue;
    NextFenceValue = 1;
    PendingSignals.Clear();

    if (pDevice && pCommandQueue)
    {
        pDevice->AddRef();
        pCommandQueue->AddRef();
        WaitEvent = CreateEvent(0, FALSE, FALSE, 0);
        if (FAILED(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence))))
            pFence = 0;
    }
}

void RenderSync::BeginFrame()
{
    Render::RenderSync::BeginFrame();
}

bool RenderSync::EndFrame()
{
    return Render::RenderSync::EndFrame();
}

UInt64 RenderSync::SetFence()
{
    // Records a fence value for the work currently being recorded. The value is
    // signaled on the command queue by FlushPendingSignals(), once the command
    // list has actually been submitted - signaling here would mark work as
    // complete before the GPU has even been asked to execute it.
    if (!pCommandQueue || !pFence)
        return 0;

    UInt64 fenceValue = NextFenceValue++;
    PendingSignals.PushBack(fenceValue);
    return fenceValue;
}

void RenderSync::FlushPendingSignals()
{
    if (!pCommandQueue || !pFence)
        return;

    for (UPInt i = 0; i < PendingSignals.GetSize(); i++)
        pCommandQueue->Signal(pFence, PendingSignals[i]);
    PendingSignals.Clear();
}

bool RenderSync::IsPending(FenceType waitType, UInt64 handle, const FenceFrame& parent)
{
    SF_UNUSED2(waitType, parent);
    if (!pFence || !handle)
        return false;
    return pFence->GetCompletedValue() < handle;
}

void RenderSync::WaitFence(FenceType waitType, UInt64 handle, const FenceFrame& parent)
{
    SF_UNUSED2(waitType, parent);
    if (!pFence || !handle || !WaitEvent)
        return;
    if (pFence->GetCompletedValue() < handle)
    {
        pFence->SetEventOnCompletion(handle, WaitEvent);
        WaitForSingleObject(WaitEvent, INFINITE);
    }
}

}}} // namespace Scaleform::Render::D3D12
