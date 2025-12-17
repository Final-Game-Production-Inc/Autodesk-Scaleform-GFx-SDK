/**********************************************************************

PublicHeader:   Render
Filename    :   Render_Sync.cpp
Content     :   Common implementation for GPU/CPU synchronization (fencing).
Created     :   Oct 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render/Render_Sync.h"
#include "Kernel/SF_Debug.h"

namespace Scaleform { namespace Render { 

// *** FenceImpl
FenceImpl::FenceImpl() :
    RSContext(0), Parent(0), APIHandle(0), FenceID(0)
{

}
FenceImpl::FenceImpl( RenderSync* ctx, FenceFrame* parent, UInt64 apiHandle, UInt64 id ) : 
    RSContext(ctx), Parent(parent), APIHandle(apiHandle), FenceID(id)
{
}

bool FenceImpl::IsPending(FenceType waitType) const
{
    // If the parent has been stripped, it means the entire frame has already passed.
    if (!Parent)
        return false;
    return RSContext->IsPending(waitType, APIHandle, *Parent);
}

void FenceImpl::WaitFence(FenceType waitType) const
{
    // If the parent has been stripped, it means the entire frame has already passed.
    if ( !Parent ) return; 

    RSContext->WaitFence(waitType, APIHandle, *Parent);
}

const FenceFrame* FenceImpl::GetFrame() const
{
    return Parent;
}

bool FenceImpl::operator>( const FenceImpl& fence ) const
{
    return FenceID > fence.FenceID;
}


//*** Fence
void Fence::Release()
{
    if ( --RefCount > 0 )
        return;

    // Destruction.
    if ( HasData )
    {
        RenderSync* ctx = Data->RSContext;
        RSContext->ReleaseFence(Data->APIHandle);
        ctx->FenceImplAlloc.Free(Data);
        ctx->FenceAlloc.Free(this);
    }
    else
    {
        RSContext->FenceAlloc.Free(this);
    }
}


//*** FenceFrame

FenceFrame::~FenceFrame()
{
    FenceHandleArray::Iterator item = Fences.Begin();
    for ( item = Fences.Begin(); item != Fences.End(); ++item )
    {
        // Clear the handle's fence pointer, because it has passed, and replace it
        // with the RSContext pointer (so it can free its data at a later time).
        if ( (*item)->HasData )
        {
            (*item)->HasData = 0;
            RSContext->ReleaseFence((*item)->Data->APIHandle);
            RSContext->FenceImplAlloc.Free((*item)->Data);
            (*item)->RSContext = RSContext;
        }
    }
    Fences.Clear();
    FrameEndFence = 0;
}

//*** RenderSync

RenderSync::~RenderSync()
{
    ReleaseOutstandingFrames();
}

void RenderSync::BeginFrame()
{
    WithinFrame = true;
    FenceFrame* frame = FenceFrameAlloc.Alloc();
    frame->RSContext = this;
    FenceFrames.PushBack(frame);
    OutstandingFrames++;
    
    // If we reach 100 outstanding, something may have gone wrong. This could potentially 
    // happen if many DrawableImage operations are performed without flushing.
    SF_DEBUG_WARNONCE1(OutstandingFrames >= 100, "RenderSync outstanding frames is %d. "
        "This may be the result of many BitmapData operations, extremely high framerate,"
        "or HAL::EndFrame is not being called appropriately.", OutstandingFrames);
}

bool RenderSync::EndFrame()
{
    // Insert an end fence onto the current frame.
    if ( FenceFrames.IsEmpty() )
        return false;
    FenceFrames.GetLast()->FrameEndFence = InsertFence();

    // Check if we are close to wrap around. 
    bool wrapped = CheckWraparound(FenceFrames.GetLast()->FrameEndFence->GetFencePtr()->APIHandle);

    // Go through previous fences, and see if we can reclaim them. Do not
    // reclaim the final frame, even if has passed.
    FenceFrame* passFrame = FenceFrames.GetFirst();
    while ( !FenceFrames.IsLast(passFrame) )
    {
        // If we haven't passed this frame, stop.
        if ( !passFrame->FrameEndFence || passFrame->FrameEndFence->IsPending(FenceType_Fragment) )
            break;
        FenceFrame * next = passFrame->pNext;
        passFrame->RemoveNode();
        FenceFrameAlloc.Free(passFrame);
        passFrame = next;
        OutstandingFrames--;
    }

    // If we wrapped around, mark all frames as wrapped.
    if ( wrapped )
    {
        passFrame = FenceFrames.GetFirst();
        while ( !FenceFrames.IsNull(passFrame))
        {
            passFrame->WrappedAround = true;
            passFrame = passFrame->pNext;
        }
    }
    WithinFrame = false;
    return true;
}

Fence* RenderSync::InsertFence()
{
    SF_DEBUG_ASSERT(WithinFrame, "RenderSync::InsertFence - not within fence frame.");
    if (FenceFrames.IsEmpty())
        return 0;
    UInt64 apiHandle = SetFence();
    FenceImpl* pf = new(FenceImplAlloc.Alloc()) FenceImpl(this, FenceFrames.GetLast(), apiHandle, ++NextFenceID );
    Ptr<Fence> fhi = *FenceAlloc.Alloc();
    fhi->HasData = 1;
    fhi->Data = pf;
    FenceFrames.GetLast()->Fences.PushBack(fhi);
    return fhi;
}

void RenderSync::ReleaseOutstandingFrames()
{
    // Kill all outstanding frames.
    FenceFrame* frame = FenceFrames.GetFirst();
    while ( !FenceFrames.IsNull(frame) )
    {
        FenceFrame * next = frame->pNext;
        frame->RemoveNode();
        FenceFrameAlloc.Free(frame);
        frame = next;
        OutstandingFrames--;
    }
    SF_ASSERT(OutstandingFrames == 0);
}

}}; // Scaleform::Render