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
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Uncopyable.h"

#include "Render/Render_Stats.h"

namespace Scaleform { namespace Render { 

class FenceImpl;
class Fence;
class FenceFrame;
class RenderSync;
class HAL;
class MemoryManager;

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
class FenceImpl : public Unassignable
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
    bool WaitFence(FenceType waitType) const;

    // Gets the frame that this fence was inserted in. If 0, the frame has been passed.
    const FenceFrame* GetFrame() const;

    // Used to determine fence insertion order.
    bool operator> ( const FenceImpl& fence ) const;

    // Gets the hardware API's fence handle.
    UInt64 GetAPIHandle() const { return APIHandle; }

protected:

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
    bool IsPending(FenceType waitType) const 
    { 
        return ( HasData && Data ) ? Data->IsPending(waitType) : false; 
    }

    // Waits for this fence to pass.
    bool WaitFence(FenceType waitType) const 
    { 
        if ( HasData && Data ) 
            return Data->WaitFence(waitType); 
        return true;
    }

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
// Acts as an interface to any resource that requires GPU fencing.
class FenceResource
{
    friend class RenderSync;
public:
    bool IsPending(FenceType waitType) const    { return GPUFence && GPUFence->IsPending(waitType); }
    bool WaitFence(FenceType waitType) const    { return !GPUFence || GPUFence->WaitFence(waitType); }
    bool HasFence() const                       { return GPUFence != 0; }
    Fence* GetFence()                           { return GPUFence; }
protected:
    Ptr<Fence>  GPUFence;
};

// Helper class to destroy resources when their fences are passed. 'ResourceType' must implement FenceResource interface, and FreeResource method.
template<class ResourceType>
class FenceResourceDestroyList
{
public:
    ~FenceResourceDestroyList()
    {
        UPInt remainingItems = ProcessDestructionList(true); SF_UNUSED(remainingItems);
        SF_DEBUG_ASSERT1(remainingItems == 0, "FenceResourceDestroyList destroyed, but not all resources could be destroyed (%zd remaining)", remainingItems);
    }
    void PushBack(const ResourceType& e)
    {
        PendingFreeList.PushBack(e);
    }
    bool IsEmpty() const
    {
        return PendingFreeList.IsEmpty();
    }

    // Removes all entries from the list, whether their fences have passed or not. Used in GLES lost context,
    // because the resources no longer exist.
    void Clear()
    {
        PendingFreeList.Clear();
    }

    // Calls 'FreeResource' on resources whose fences have passed. If 'forceWait' is true, CPU will wait for the fence to pass, if possible.
    // Returns the number of resources still waiting for destruction.
    UPInt ProcessDestructionList(bool forceWait = false)
    {
        for (unsigned i = 0; i< PendingFreeList.GetSize();)
        {
            ResourceType& entry = PendingFreeList[i];
            bool freeResource = true;
            if (entry.IsPending(FenceType_Fragment))
            {
                if (!forceWait)
                {
                    // Skip this one, not ready to be freed yet.
                    freeResource = false;
                    i++; 
                }
                else
                {
                    // Wait if requested.
                    if (!entry.WaitFence(FenceType_Fragment))
                    {
                        // Wait failed (unsubmitted command buffer?).
                        freeResource = false;
                        SF_DEBUG_WARNING(1, "Requested WaitFence on resource but WaitFence returned false.");
                        i++;
                    }
                }
            }

            if (freeResource)
            {
                // Free and remove.
                entry.FreeResource();
                PendingFreeList.RemoveAt(i);
            }
        }

        return PendingFreeList.GetSize();
    }

protected:
    typedef ArrayConstPolicy<8, 8, false>                                               KillListArrayPolicy;
    typedef ArrayLH<ResourceType, StatRender_RenderPipeline_Mem, KillListArrayPolicy>   ResourceArrayType;

    ResourceArrayType   PendingFreeList;
};

//---------------------------------------------------------------------------------------
// Rendering synchronization class.
class RenderSync : public RefCountBase<RenderSync,Stat_Default_Mem>
{
    friend class FenceFrame;
    friend class FenceImpl;
    friend class Fence;
public:

    // Identifies the type of submission that causes Fence objects to be updated (by the GPU).
    enum SubmissionType
    {
        SubmissionType_Invalid,     // Used to indicate uninitialized system.
        SubmissionType_DrawCall,    // Per draw call. Use this on systems which continually process the command buffer, and reclaiming resources immediately is important.
        SubmissionType_Scene,       // Per scene. Use this when low latency on freeing resources is not required.
        SubmissionType_Count        // Number of submission types.
    };

    RenderSync();
    virtual ~RenderSync();

    // Sets the render HAL context. NOTE: 'phal' must own this object. Use phal = NULL to shutdown the system.
    // Derived override queries the HAL for flags, or any other required interfaces (device, etc).
    virtual bool SetContext(Render::HAL* phal);

    // Signals that a new frame has begun. May be overridden, but base class function should also be called.
    virtual void BeginFrame();

    // Signals that the frame has ended. May be overridden, but base class function should also be called.
    virtual bool EndFrame();

    // Write a fence into the command buffer.
    Fence* InsertFence();

    // Methods that must be overridden per hardware implementation.
    virtual void   KickOffFences(FenceType waitType) = 0;

    // Add the fence pointer to an internal list of that needs updating after a submission. 
    // They are updated depending on the fence 'updateMethod' and calls to NotifySubmisson. 
    void AddFenceResource(FenceResource* fenceResource);
    void AddFenceResource(Ptr<Fence>& fence);
    void NotifySubmission(SubmissionType type);

protected:

    // Methods that must be overridden per hardware implementation.
    virtual bool   IsPending(FenceType waitType, UInt64 handle, const FenceFrame& parent) = 0;
    virtual bool   WaitFence(FenceType waitType, UInt64 handle, const FenceFrame& parent) = 0;
    virtual UInt64 SetFence() = 0;

    // Can be overridden on platforms whose fences may wraparound. If the handle is high enough,
    // wraparound should be performed on the API handle values. Return true if wraparound occurs.
    virtual bool   CheckWraparound(UInt64 handle) { SF_UNUSED(handle); return false; }

    // Can be overridden on platforms that require a retain/release for the API specific fence objects.
    virtual void   AddRefFence(UInt64 apiHandle) { SF_UNUSED(apiHandle); }
    virtual void   ReleaseFence(UInt64 apiHandle) { SF_UNUSED(apiHandle); };

    // Destroys all frames, regardless if they are passed or not (useful with reset/shutdown).
    void           ReleaseOutstandingFrames();

    static const UInt64         INVALID_FENCE_ID = UInt64(~0);   // Used to denote a fence which has not been issued yet.
    List<FenceFrame>            FenceFrames;            // The list of fence frames currently tracked by the system.
    ListAllocLH<FenceFrame>     FenceFrameAlloc;        // Paged allocator for FenceFrames objects.
    ListAllocLH_POD<FenceImpl>  FenceImplAlloc;         // Paged allocator for FenceImpl objects.
    ListAllocLH<Fence>          FenceAlloc;             // Paged allocator for Fence objects.
    Ptr<Fence>                  SubmissionDummyFence;   // Data assigned to FenceResource/Fence when added via AddFenceResource.
    UInt64                      NextFenceID;            // The value of the next fence ID.
    unsigned					OutstandingFrames;      // The number of outstanding FenceFrames
    bool                        WithinFrame;            // Whether the RenderSync is between a BeginFrame/EndFrame pair.
    SubmissionType              SubmissionUpdateMethod; // The type of submission that causes the GPU to start processing.
    Render::HAL*                pHAL;                   // The HAL object. Used to query config parameters. Not a smart pointer, as it should own 'this'.
    Render::MemoryManager*      pMemoryManager;         // MemoryManager. May be used to allocate queries, depending on the platform.
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
    FenceWrapperList(unsigned poolSize = 4);
    virtual ~FenceWrapperList();

    // Allocates reserve queries.
    virtual void Initialize();

    // Destroys queries in the free list. It is assumed that there are no other outstanding FenceWrapper objects.
    virtual void Shutdown();

    // Allocates an FenceWrapper, or returns an unused one from the pool. Handles reference counting internally.
    FenceWrapper* Alloc();

    // Free the given query. If the query is otherwise unused, it is added to the free list.
    void Free(FenceWrapper* query);

protected:
    virtual FenceWrapper* allocateWrapper() = 0;

    List<FenceWrapper>    WrapperList;
    unsigned              ReservePoolSize;
    unsigned              AllocatedWrappers;
    bool                  Initialized;
};

}};

#endif // INC_SF_Render_Sync_H
