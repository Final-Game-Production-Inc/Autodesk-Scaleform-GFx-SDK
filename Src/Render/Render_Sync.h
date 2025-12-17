/**********************************************************************

PublicHeader:   Render
Filename    :   Render_Sync.h
Content     :   Common implementation for GPU/CPU synchronization (fencing).
Created     :   Oct 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_Render_Sync_H
#define INC_SF_Render_Sync_H

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_ListAlloc.h"

#include "Render/Render_Stats.h"

namespace Scaleform { namespace Render { 

class FenceImpl;
class Fence;
class FenceFrame;
class RenderSync;

// Represents the type of fence. For most platforms, these are equivalent, because fragment and vertex 
// processing happen in parallel; fences are not 'passed' until both have completed. However, on PSVITA 
// for example, there are separate vertex and fragment passes, so they are not equivalent on those systems.
enum FenceType
{
    FenceType_Vertex,     // Vertex processing fence.
    FenceType_Fragment    // Fragment processing fence.
};

//---------------------------------------------------------------------------------------
// Controls usage of GPU fence objects.
class FenceImpl
{
    friend class RenderSync;
    friend class FenceFrame;
    friend class Fence;
public:
    FenceImpl();

    // apiHandle is a platform specific value, representing the value that the platform's hardware
    // API is expecting to use in fencing operations.
    FenceImpl( RenderSync* ctx, FenceFrame* parent, UInt64 apiHandle, UInt64 id );

    // Determines whether this fence has yet to be passed.
    bool IsPending(FenceType waitType) const;

    // Waits for this fence to pass.
    void WaitFence(FenceType waitType) const;

    // Gets the frame that this fence was inserted in. If 0, the frame has been passed.
    const FenceFrame* GetFrame() const;

    // Used to determine fence insertion order.
    bool operator> ( const FenceImpl& fence ) const;

    // Gets the hardware API's fence handle.
    UInt64 GetAPIHandle() const { return APIHandle; }

protected:

    // Fix assignment operator warning.
    FenceImpl& operator=(const FenceImpl& f) { SF_UNUSED(f); return *this; };

    RenderSync*  RSContext;  // Reference to implementation specific interface.
    FenceFrame*  Parent;     // Pointer to the frame in which this fence was inserted (can be 0 if the frame has completely passed).
    UInt64       APIHandle;  // Implementation specific fence data.
    UInt64       FenceID;    // Implementation non-specific ordering number.
};

//---------------------------------------------------------------------------------------
// A Fence is the structure used by HAL items keeping track of their required GPU fences.
// This includes MeshCacheItems, as well as GlyphCacheSlots. The data pointer may be nullified
// during RenderSync::EndFrame, when entire frame(s) of fences are reclaimed. 
class Fence
{
    friend class RenderSync;
    friend class FenceFrame;
public:

    // Determines whether this fence has yet to be passed.
    bool IsPending(FenceType waitType) const { return ( HasData && Data ) ? Data->IsPending(waitType) : false; }

    // Waits for this fence to pass.
    void WaitFence(FenceType waitType) const { if ( HasData && Data ) Data->WaitFence(waitType); }

    // Determines fence ordering. This may not follow strict ordering, when both
    // handles have lost their fence pointers. This is inconsequential however, because
    // this means that both fences have past.
    bool operator> (const Fence& fh)
    {
        if ( !Data )
            return false;
        if ( !fh.Data )
            return true;
        return (*Data > *fh.Data);
    }

    // Gets a pointer to the fence object.
    const FenceImpl* GetFencePtr() const { return HasData ? Data : 0; } 

    // These only be created by RenderSync.
    Fence()              : Data(0), RefCount(1), HasData(0) { }
    Fence(FenceImpl * f) : Data(f), RefCount(1), HasData(1) { }

    // AddRef/Release - not thread safe, should only be called on the render thread.
    void AddRef() { RefCount++; }
    void Release();

private:
    union 
    {
        FenceImpl* Data;         // Pointer to the actual fence (if HasData is non-zero).
        RenderSync*  RSContext;  // Reference to implementation specific interface (if HasData is zero).
    };
    UInt16 RefCount;           // Object reference count.
    UInt8  HasData;            // Whether this object has a pointer to the actual fence, or the RenderSync.
};


//---------------------------------------------------------------------------------------
// A collection of fences inserted on a particular frame.
class FenceFrame : public NewOverrideBase<Stat_Default_Mem>, public ListNode<FenceFrame>
{
    friend class RenderSync;
    friend class Fence;

    // Assume we will have less than 128 fences per frame, and allocate the array up front.
    typedef ArrayConstPolicy<128, 64, true> FenceHandleArrayPolicy;
    typedef ArrayLH<Ptr<Fence>, Stat_Default_Mem, FenceHandleArrayPolicy> FenceHandleArray;

public:
    FenceFrame() : ListNode<FenceFrame>(), RSContext(0), WrappedAround(false), FrameEndFence(0) { }
    ~FenceFrame();

    // Returns the end-of-frame fence. Can be 0, if the frame has not ended yet.
    const Fence* GetFenceEndFrame() const { return FrameEndFence; }

    // Returns whether the API handles have wrapped around since this frame ended.
    bool IsWrappedFrame() const { return WrappedAround; }

protected:    

    RenderSync*         RSContext;      // Reference to implementation specific interface.
    bool                WrappedAround;  // If true, subsequent frames have wrapped around implementation specific fence IDs.
    FenceHandleArray    Fences;         // List of fences that were inserted in this frame.
    Ptr<Fence>          FrameEndFence;  // The fence that indicates the end of this frame.
};

//---------------------------------------------------------------------------------------
// Rendering synchronization class.
class RenderSync : public RefCountBase<RenderSync,Stat_Default_Mem>
{
    friend class FenceFrame;
    friend class FenceImpl;
    friend class Fence;
public:

    RenderSync( ) : NextFenceID(0), OutstandingFrames(0), WithinFrame(false) { };
    virtual ~RenderSync();

    // Signals that a new frame has begun. May be overridden, but base class function should also be called.
    virtual void BeginFrame();

    // Signals that the frame has ended. May be overridden, but base class function should also be called.
    virtual bool EndFrame();

    // Write a fence into the command buffer.
    Fence* InsertFence();

    // Methods that must be overridden per hardware implementation.
    virtual void   KickOffFences(FenceType waitType) = 0;

protected:

    // Methods that must be overridden per hardware implementation.
    virtual bool   IsPending(FenceType waitType, UInt64 handle, const FenceFrame& parent) = 0;
    virtual void   WaitFence(FenceType waitType, UInt64 handle, const FenceFrame& parent) = 0;
    virtual UInt64 SetFence() = 0;

    // Can be overridden on platforms whose fences may wraparound. If the handle is high enough,
    // wraparound should be performed on the API handle values. Return true if wraparound occurs.
    virtual bool CheckWraparound(UInt64 handle) { SF_UNUSED(handle); return false; }

    // Can be overridden on platforms that require a release for the API specific fence objects.
    virtual void   ReleaseFence(UInt64 apiHandle) { SF_UNUSED(apiHandle); };

    // Destroys all frames, regardless if they are passed or not (useful with reset/shutdown).
    void           ReleaseOutstandingFrames();

    List<FenceFrame>            FenceFrames;        // The list of fence frames currently tracked by the system.
    ListAllocLH<FenceFrame>     FenceFrameAlloc;    // Paged allocator for FenceFrames objects.
    ListAllocLH_POD<FenceImpl>  FenceImplAlloc;     // Paged allocator for FenceImpl objects.
    ListAllocLH<Fence>          FenceAlloc;         // Paged allocator for Fence objects.
    UInt64                      NextFenceID;        // The value of the next fence ID.
    unsigned					OutstandingFrames;	// The number of outstanding FenceFrames
    bool                        WithinFrame;        // Whether the RenderSync is between a BeginFrame/EndFrame pair.
};

// FenceWrapper is intended to be overridden in derived implementations. It is useful if the platform performs
// better with pooled fence/sync/query objects, so there is minimal allocation of these objects.
class FenceWrapper : public ListNode<FenceWrapper>, public RefCountBase<FenceWrapper, StatRender_Mem>
{
public:
    FenceWrapper() : ListNode<FenceWrapper>() { }
};

// List class which manages a pool of FenceWrapper instances. This class is optionally used by derived RenderSync implementations.
class FenceWrapperList
{
public:
    FenceWrapperList(unsigned poolSize = 4) : ReservePoolSize(poolSize) { }
    virtual ~FenceWrapperList() { }

    // Allocates reserve queries.
    virtual void Initialize()
    {
        for (unsigned i = 0; i < ReservePoolSize; ++i)
        {
            FenceWrapper* query = allocateWrapper();
            WrapperList.PushBack(query);
        }
    }

    // Destroys queries in the free list. It is assumed that there are no other outstanding FenceWrapper objects.
    virtual void Shutdown()
    {
        while (!WrapperList.IsEmpty())
        {
            FenceWrapper* wrapper = WrapperList.GetFirst();
            wrapper->RemoveNode();
            wrapper->Release();
        }
    }

    // Allocates an FenceWrapper, or returns an unused one from the pool. Handles reference counting internally.
    FenceWrapper* Alloc()
    {
        if (WrapperList.IsEmpty())
        {            
            FenceWrapper * query = allocateWrapper();
            // Extra reference to track when it should be moved to free list.
            query->AddRef(); 
            return query;
        }
        FenceWrapper* query = WrapperList.GetFirst();
        query->RemoveNode();
        query->AddRef();
        return query;
    }

    // Free the given query. If the query is otherwise unused, it is added to the free list.
    void Free(FenceWrapper* query)
    {
        if (query)
        {
            query->Release();
            if (query->GetRefCount() == 1)
                WrapperList.PushBack(query);
        }    
    }

protected:
    virtual FenceWrapper* allocateWrapper() = 0;

    List<FenceWrapper>    WrapperList;
    unsigned              ReservePoolSize;
};

}};

#endif // INC_SF_Render_Sync_H
