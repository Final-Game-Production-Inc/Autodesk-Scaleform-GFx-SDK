/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Context.h
Content     :   Threaded rendering context used by the 2D renderer.
Created     :   August 17, 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Context_H
#define INC_SF_Render_Context_H

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_List.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_AmpInterface.h"
#include "Kernel/SF_Debug.h"

#include "Render_Containers.h"
#include "Render_Stats.h"
#include "Render_Constants.h"
#include "Render_ThreadCommandQueue.h"

//--------------------------------------------------------------------
// ***** Render::Context

// Context is a thread-safe rendering support system that manages lifetime
// of a rendering tree that can be modified in one thread while it is being used
// for rendering in another. The rendering tree consists of Context::Entry derived
// nodes that hold rendering information and references to other nodes. Entry data
// can be accessed and modified through their custom access methods such as
// GetMatrix/SetMatrix, AddChild, etc.
//
// The main feature of the context is its ability to take a snapshot of the
// rendering tree by calling Context::Capture(). This method is typically called
// by the Advance thread after it is done modifying the tree nodes. After the first
// Capture has taken place, the rendering thread can get access to the last captured
// snapshot by calling NextCapture(), after which point it can traverse the tree and
// use it for rendering. In the mean time, the Advance thread it free to proceed
// modifying the rendering tree for the next frame capture. Since the Context maintains
// multiple logically independent snapshots of the tree, both threads can work
// without interfering with each other.
//
//
// Public interface includes the following:
//   - Context class, created directly.
//   - Context::Entry, Context::RenderNotify, Context::EntryChange, Context::ChangeBuffer.

namespace Scaleform { namespace Render {

class TreeCacheNode;

namespace ContextImpl {
    class Context;
    class ContextCaptureNotify;
    template <class C, class B> class ContextData_ImplMixin;
    template <class C> class DisplayHandle;
    template<class C> struct NonlocalCloneArg;
};
    using ContextImpl::Context;
    using ContextImpl::ContextCaptureNotify;
    using ContextImpl::ContextData_ImplMixin;
    using ContextImpl::DisplayHandle;
    using ContextImpl::NonlocalCloneArg;
}};

namespace Scaleform { class Event; }

//--------------------------------------------------------------------
// Context Implementation namespace
namespace Scaleform { namespace Render { namespace ContextImpl {

// Implementation helpers.
struct EntryPage;
struct SnapshotPage;
class Snapshot;
class Entry;
class EntryData;
class EntryChange;
class Context;
class RTHandle;
class RenderNotify;


//--------------------------------------------------------------------
// ContextLock is allocated in a separate global object so that it can be use
// to synchronize RTHandle use.

class ContextLock : public RefCountBase<ContextLock, StatRender_Context_Mem>
{
public:
    Lock     LockObject;
    // pContext is only accessible during lock; its cleared out once
    // context begins shutdown (~Context).
    Context* pContext; 

    ContextLock(Context* context) : pContext(context)
    { }
};


//--------------------------------------------------------------------

// RTHandle enables simplified thread-safe passage of nodes from main to
// render thread, without requiring synchronization with Context.
class RTHandle
{
    friend class Context;
    friend class Entry;

    enum HandleState
    {
        State_PreCapture,
        State_Valid,
        State_Dead
    };
    
    // Thread-safe ref-counting; allocated from global heap.
    struct HandleData : public RefCountBase<HandleData, StatRender_Context_Mem>,
                        public ListNode<HandleData>
    {
        Ptr<ContextLock> pContextLock;
        HandleState      State;
        Entry*           pEntry;
        // List node 
        HandleData(Entry* entry, Context* context);
        ~HandleData();
    };

    Ptr<HandleData> pData;

public:
    RTHandle() { }
    RTHandle(Entry* entry);
    RTHandle(const RTHandle& other) : pData(other.pData) { }
    ~RTHandle();
    
    void operator = (const RTHandle& other)  { pData = other.pData; }
    
    bool operator == (const RTHandle& other) const { return pData == other.pData; }

    void    Clear() { pData.Clear(); }
    // Obtain Render::Context associated with this handle.
    // This function doesn't check for Context lifetime.
    Context* GetContext_Unsafe() const { return pData ? pData->pContextLock->pContext : 0; }


    // *** RenderThread API

    // Captures entry state for the frame, returning 'true' if the node
    // can be rendered and 'false' if it is no longer alive.
    bool    NextCapture(RenderNotify* render);

    Entry*  GetRenderEntry() const;
};

// DisplayHandle is a context entry handle intended to pass values of
// a specific node to the rendering thread, where it is typically passed
// to Renderer2D::Display method. DisplyHandle implementation is essentially a
// "weak reference" in that NextCapture call on the handle will fail if
// Entry/TreeNode was released.

template<class C>
class DisplayHandle : public RTHandle
{
public:
    DisplayHandle() { }
    DisplayHandle(C* entry) : RTHandle(entry) { }
    DisplayHandle(const RTHandle& other) : RTHandle(other) { }
    
    void operator = (const DisplayHandle& other)
    { RTHandle::operator = (other); }

    bool operator == (const DisplayHandle<C>& other) const
    { return RTHandle::operator == (other); }

    // Captures entry state for the frame, returning 'true' if the node
    // can be rendered and 'false' if it is no longer alive.
    C*  GetRenderEntry() const { return (C*)RTHandle::GetRenderEntry(); }
};


//--------------------------------------------------------------------
// *** Context::Entry

// Entry is a base class for all of the context-allocated entries
// used for rendering nodes; users should derive all of the rendering tree
// items from this class.
//
// The main feature of this class is that it supports copy-on-write access,
// with data being stored in a separately EntryData. Users are expected to 
// implement data accessors by going through the GetReadOnlyData and
// GetWritableData APIs. GetWritableData will in generate replicate the
// data, allowing background rendering threads to access the tree.

class Entry
{
protected:
    friend struct EntryListAccessor;
    //friend class List<Entry>;   
    friend class Context;
    friend class Snapshot;
    friend class RTHandle;
    friend struct RTHandle::HandleData;

    // Allow Entry to exist in linked lists.
    // Important: pPrev must come before pNext for compatibility
    //            with ListNode<>!
    union {
        Entry*      pPrev;
        EntryChange*pChange;
    };
    union {
        Entry*      pNext;
        UPInt       RefCount;
    };
    

    // Pointer to original front-allocated EntryData.
    // Low bit stores a "has RTHandle" mark, which is mased out on access.
    EntryData*      pNative; 
    TreeCacheNode*  pRenderer;
    // Parent node or null.
    Entry*          pParent;

    struct PropagateNode : public ListNode<PropagateNode>
    {
        inline Entry* GetEntry();
        bool IsAdded() const { return pPrev != 0;}
        void Clear()         { pPrev = pNext = 0; }        
    };


    // Propagation chain; links next node that needs propagation.
    // TBD: Optimization - This could in theory be merged into pChange
    // by using lower bit pointer (and a member in pChange if it exists).
    PropagateNode   PNode;

    void            destroyHelper();
    void            addToPropagateImpl();

    EntryPage*   getEntryPage() const;
    Snapshot*    getSnapshot() const;
    Context*     getContext() const;

    void            setHasRTHandle()    { pNative = (EntryData*)(UPInt(pNative)|1); }
    void            clearHasRTHandle()  { pNative = GetNative(); }
    bool            hasRTHandle() const { return (UPInt(pNative) & 1) != 0; }

    // 0xBAD pRenderer is a marker for a destroyed Entry; it shouldn't happen
    // unless dead node pointer was passed from Advance thread.
    static TreeCacheNode*    badRenderData() { return (TreeCacheNode*)0xbad; }

protected:
    EntryData*      getWritableData(unsigned changeBits);

public:

    inline const EntryData* GetReadOnlyDataBase() const;
    inline const EntryData* GetDisplayDataBase() const;
    
    inline void Init(EntryData* pnative, EntryChange *pchange)
    {
        RefCount = 1;
        pNative  = pnative;
        pRenderer= 0;
        pChange  = pchange;
        pParent  = 0;
        PNode.Clear();        
    }


    inline EntryChange* GetChange() const           { return pChange; }
    inline void         SetChange(EntryChange* p)   { pChange = p; }
    inline EntryData*   GetNative() const           { return (EntryData*)(((UPInt)pNative)&~UPInt(1)); }
    inline void         ClearNative()               { pNative = 0; }
   
    inline TreeCacheNode* GetRenderData() const
    { SF_ASSERT(pRenderer != badRenderData()); return pRenderer; }
    inline void         SetRenderData(TreeCacheNode* p)
    { SF_ASSERT(p != badRenderData()); pRenderer = p; }
    inline TreeCacheNode* GetRenderDataClean() const
    { return (pRenderer != badRenderData()) ? pRenderer : 0; }
    inline void         SetRenderDataAsBAD()
    { pRenderer = badRenderData();}

    // Determine node depth by scanning its parent chain.
    unsigned        CalcDepth() const
    {
        unsigned depth = 0;
        for (const Entry* p = this; p->pParent != 0; p = p->pParent)
            depth++;
        return depth;
    }        

    inline void         AddToPropagate() { if (!PNode.IsAdded()) addToPropagateImpl(); }
    inline Entry*       GetParent() const { return pParent; }
    inline void         SetParent(Entry* parent) { pParent = parent; }

    // Reference counting. Destroy may be delayed with entry being placed in a list instead.
    inline void         AddRef()      { RefCount++; }        
    inline void         Release()     { if (--RefCount == 0) destroyHelper(); }  

    Context*  GetContext() const; // non-inline public implementation of getContext
};


struct EntryListAccessor
{
    static void SetPrev(Entry* self, Entry* what)  { self->pPrev = what; }
    static void SetNext(Entry* self, Entry* what)  { self->pNext = what; }
    static const Entry* GetPrev(const Entry* self) { return self->pPrev; }
    static const Entry* GetNext(const Entry* self) { return self->pNext; }
    static       Entry* GetPrev(Entry* self)       { return self->pPrev; }
    static       Entry* GetNext(Entry* self)       { return self->pNext; }
};

// Use a custom EntryList to avoid warning:
// "dereferencing type-punned pointer will break strict-aliasing rules"..
typedef List2<Entry, EntryListAccessor> EntryList;
//typedef List<Entry> EntryList;

// NonlocalCloneArg is a non-local context copy constructor argument,
// used in EntryData/TreeNode constructor to do selective copy.
// Selective copy is necessary when cloning a the tree for DrawableImage.
template<class C>
struct NonlocalCloneArg
{
    const C* pC;
    NonlocalCloneArg(const C& c) : pC(&c) { }
    NonlocalCloneArg(const NonlocalCloneArg& s) : pC(s.pC) { }
    const C* operator -> () const { return pC; }
};


//--------------------------------------------------------------------
// *** Context::EntryData

// EntryData is a base class for all Entry data items. Contents of this
// class contain frame-state information such as the current Matrix, effects
// applied and pointers to child nodes.
//
// EntryData class is allocated internally as a part of CreateEntry<> call and
// is maintained by the context. Write access will generally replicate the
// EntryData by going through the virtual function defined in it. 

class EntryData
{        
public:
    
    enum EntryType
    {
        ET_Base,
        ET_Root,
        ET_Container,
        ET_Shape,
        ET_Mesh,
        ET_Text
    };

    EntryData(EntryType type = ET_Base, UInt16 flags = 0)
        : Type((UInt16)type), Flags(flags) { }    
    EntryData(const EntryData& src)
        : Type(src.Type), Flags(src.Flags) { }
    // "Copy constructor" for external context cloning. Mask bits must be handled separately.
    EntryData(NonlocalCloneArg<EntryData> src)
        : Type(src->Type), Flags(src->Flags & ~(UInt16)(NF_HasMask|NF_MaskNode)) { }
    virtual ~EntryData();

    inline EntryType    GetType() const { return (EntryType)Type; }

    /*  Context Data Copy protocol

    EntryData needs to use special semantics for copies of pointers to
    child / dependent Entry nodes. Specifically:
    - CopyConstruct, CopyTo and Destroy should NOT AddRef or Release
      any Entry pointers; instead is should just copy them by value.
    - ReleaseNode should be implemented to release those pointers without
      clearing them (hence is is Const).
    - Any node pointer reassignment within the object (after copy on write
      due to change) does need to properly AddRef and Release the associated
      pointers.

    Such semantics is more efficient because (a) it reduces overall ref-counting
    overhead and (b) allows RefCount to be non-thread-safe, since all RefCount
    manipulations are known to take place on Advance thread that manipulates the tree.
    With this logic, ref-count ownership is effectively transfered to the
    last copy created; furthemore, it allows tree destroy to take place immediately.

    If EntryData holds any shared data Ptr<>s, those MUST be properly AddRefed
    and releases on copy in Thread-Safe manner.
    */

    //  virtual UPInt       GetSize() const                      = 0;
    virtual EntryData*  ConstructCopy(LinearHeap& heap) const= 0;
    virtual void        CopyTo(void *pdest) const            = 0;
    virtual void        ReleaseNodes() const { }
    virtual void        Destroy()                            = 0;

    // PropagateUp - provides data propagation support up the tree,
    // used specifically for bounds propagation.
    //  - Called on a parent item when one of child items have changed in a
    //    way that affects us.
    //  - Should return 'false' if no propagation to our parent is
    //    necessary; 'true' if it is.
    // Item is const because if a change is made, copy-on-write would
    // take place.
    virtual bool        PropagateUp(Entry* entry) const
    { SF_UNUSED(entry); return false; }


protected:
    UInt16      Type;
public:
    UInt16      Flags; // Visible, etc
};


//--------------------------------------------------------------------
// *** Context::EntryChange

// EntryChange is recorded every time write modification is made to the node,
// allowing change list to be passed to the renderer thread. Multiple changes
// to the same node will have only change item; however, change bit flags
// will be accumulated based on what changes took place. If pNode is NULL,
// change entry should be ignored.

class EntryChange
{
public:
    Entry*            pNode;

    union {
        unsigned      ChangeBits;
        EntryChange*  pNextFreeNode;
    };

    bool IsNewNode() const { return (ChangeBits & Change_Context_NewNode) != 0; }
};

// ChangeBuffer is used to hold EntryChange items when they are accessed by
// the rendering thread.
typedef PagedItemBuffer<EntryChange, (0x400-16)/sizeof(EntryChange)> ChangeBuffer;



//--------------------------------------------------------------------
// Context rendering interface, notifies renderer of changes.
class RenderNotify
{
public:
    struct ContextNode : public ListNode<ContextNode>
    {
        Context* pContext;
        ContextNode(Context* context) : pContext(context) { }
    };

    // Keep a list if all contexts that we are working with.
    // Accessible only from render thread.
    List<ContextNode>   ActiveContextSet;
    ThreadCommandQueue* pRTCommandQueue;

    // This command is pushed to the RenderThread queue when ActiveContextSet
    // needs servicing, typically for Context::Shutdown.
    class ServiceCommand : public ThreadCommand
    {
        RenderNotify* pNotify;
    public:
        ServiceCommand(RenderNotify* p) : pNotify(p) { }
        virtual void Execute() { pNotify->ServiceQueues(); }
    };
    ServiceCommand ServiceCommandInstance;

    
    RenderNotify(ThreadCommandQueue* commandQueue = 0)
        : pRTCommandQueue(commandQueue), ServiceCommandInstance(getThis()) { }
    virtual ~RenderNotify();

    RenderNotify* getThis() { return this; }

    // Notifies then new frame capture has taken place for a context;
    // internal implementation adds node to list. 
    // Must call base when overriding.
    virtual void    NewCapture(Context* context, bool hasChanges);
    // Called from Context::ShutdownRendering for a context.
    // Must call base when overriding.
    virtual void    ContextReleased(Context* context);

    // Services any pending shutdown requests for Context objects
    // currently associated with this renderer.
    virtual void    ServiceQueues();
    
    virtual void    EntryChanges(Context&, ChangeBuffer&, bool /*forceUpdateImages*/ = false) { }
    virtual void    EntryDestroy(Entry*) { }

    // Called from ShutdownRendering.
    virtual void    EntryFlush(Entry*) {}

    void ReleaseAllContextData();
    // Should be called from EndFrame of renderer to tell Contexts
    // of frame completion, enabling NextCapture.
    void EndFrameContextNotify();

    void PushRTServiceQueuesCommand()
    {
        SF_DEBUG_WARNING(pRTCommandQueue == 0, "Context's pRTCommandQueue is not provided.\n"
            "This may cause shutdown problems, if using multi-threaded mode. You must provide \n"
            "a valid ThreadCommandQueue object to your HAL constructor in this case.");
        if (pRTCommandQueue)
            pRTCommandQueue->PushThreadCommand(&ServiceCommandInstance);
    }
};


//--------------------------------------------------------------------
// Capture notification - used by DrawableImage
//  managed as a thread-safe list

class ContextCaptureNotify : public ListNode<ContextCaptureNotify>
{
    friend class Context;
    // Context with which we are associated, if any.
    Context* pOwnedContext;

public:
    ContextCaptureNotify() : pOwnedContext(0) { }
    virtual ~ContextCaptureNotify();

    // Notification function called when the context that we are being notified by calls Capture.
    virtual void OnCapture() { }
    
    // Notification function called when the context that we are being notified by calls NextCapture.
    virtual void OnNextCapture(RenderNotify* notify) { SF_UNUSED(notify); }

    // Notification function called when the context that we are being notified by calls Shutdown.
    // The waitFlag parameter indicates whether the owned context will wait for renderer shutdown.
    virtual void OnShutdown(bool waitFlag) { SF_UNUSED(waitFlag); }
};


//--------------------------------------------------------------------
// ***** Private Helper Classes

// EntryRef is a helper class used to simplify access to entry data by
// caching data items page and index.
class EntryRef
{
    EntryPage* pPage;
    unsigned   Index;

public:
    inline EntryRef(const Entry* pentry);

    inline EntryPage*       GetEntryPage() const       { return pPage; }
    inline unsigned         GetIndex() const           { return Index; }
    inline Snapshot*        GetSnapshot() const ;
    inline SnapshotPage*    GetSnapshotPage() const;
    inline SnapshotPage*    GetDisplaySnapshotPage() const;
    inline EntryData*       GetData() const;
    inline EntryData*       GetCleanData() const;
    inline void             SetData_KeepDestroyedBit(EntryData* arg);
    inline EntryData*       GetDisplayData() const;
    inline EntryData*&      GetDataRef() const;
    inline bool             IsDestroyed() const;
};


// EntryPage / EntryPageBase classes are used internally to hold allocated
// entries. Entry pages are organized into a linked list to allow them to
// grow and shrink without extra reallocations. Users will hold pointers to the
// internal Entry slots. Each page must be aligned to EntryPageAllocAlign to
// enable Entrys to access pages' descriptor data; this data changes when
// Context::Capture() is called.
//
// Each EntryPage has an associated SnapshotPage which holds pointers to
// the user's EntryData. Although SnapshotPages are duplicated during
// Context::Capture() call, the actual data typically is not.

struct EntryPageBase : public ListNode<EntryPageBase>
{               
    unsigned        UseCount;
    // Active Snapshot & its page.
    Snapshot*       pSnapshot;
    SnapshotPage*   pSnapshotPage;
    SnapshotPage*   pDisplaySnapshotPage;
    SnapshotPage*   pTempPage; // Used as a temporary during cleanup.
};

enum {
    EntryPageAllocAlign = 0x1000,
    EntryPageAllocSize  = 0x1000 - sizeof(void*),
    // Total size of pages.
    PageEntryCount      = (EntryPageAllocSize - sizeof(EntryPageBase)) / sizeof(Entry)
};

struct EntryPage : public EntryPageBase
{               
    Entry           Entries[PageEntryCount];

    inline void     AddEntriesToList(EntryList *plist);
    inline void     RemoveEntriesFromList(EntryList *plist);
};

// SnapshotPage holds a table of EntryData pointers for its associated EntryPage.
// SnapshotPage is a part of a Snapshot, and is duplicated every time Context::Capture()
// is called.

struct SnapshotPage : public ListNode<SnapshotPage>
{
    // NOTE: Snapshot page can die earlier then this item.
    EntryPage*      pEntryPage;

    // These point to the older and newer pipeline stages, which may be null.
    // OlderSnapshotPage is the next snapshot in the pipeline; for an Active page it will
    // point to the next captured or displaying page. For Captured page, it will point
    // to displaying page.
    SnapshotPage*   pOlderSnapshotPage;
    // NewerSnapshotPage is the next snapshot in the pipeline; for Active page is is null.
    // For Captured page it is an Active page, while for Displaying page it is Captured or
    // Active page (depending on whether a capture was grabbed by Display).
    SnapshotPage*   pNewerSnapshotPage;

    // Table of pointer to actual data items.
    EntryData*      pData[PageEntryCount];

    void            MarkAsDestroyed(unsigned index)     { *((UPInt*)(void*)&pData[index]) |= 1; }
    bool            IsDestroyed(unsigned index) const   { return *((UPInt*)(void*)&pData[index]) & 1; }
    void            SetData_KeepDestroyedBit(unsigned index, EntryData* arg)
    { pData[index] = (EntryData*) ((UPInt) arg | (((UPInt)pData[index]) & 1)); }
    EntryData*      GetCleanData(unsigned index)        { return (EntryData*)(((UPInt)pData[index]) & ~(UPInt)1); }        

    static SnapshotPage* Alloc(MemoryHeap* pheap, EntryPage* pentryPage);   
    static SnapshotPage* Clone(MemoryHeap* pheap, SnapshotPage* psource);
    void                 Free();
    // Clears EntryPage in all neighboring (older, newer) pages.
    void            ClearEntryPagePointers();
};


// EntryTable is used to manage Entry allocation; it holds a list of EntryPages
// and well as free Entrys in them.

class EntryTable
{
    friend class Context;

    Context*        pContext;
    MemoryHeap*     pHeap;
    // Full set of allocated context entries.
    List<EntryPage, EntryPageBase> EntryPages;
    // A free list of reusable nodes.
    EntryList       FreeNodes;
    // Current active snapshot used for allocations
    Snapshot*       pActiveSnapshot;

    // Allocates a page and adds its items to free list.
    bool            AllocEntryPage();   
    void            FreeEntryPage(EntryPage* ppage);

public:
    EntryTable(Context* context, MemoryHeap* heap) : pContext(context), pHeap(heap), pActiveSnapshot(0) { }

    void    SetActiveSnapshot(Snapshot* p) { pActiveSnapshot = p; }

    // Allocate/Free of ContextEntrys.
    Entry*  AllocEntry(EntryData* pdata);
    void    FreeEntry(Entry* p);


    // *** Snapshot Support

    // 1. Get current snapshot list (return it to user for storage).
    // 2. Copy the snapshot pages, setting the new set as active.
    // 3. 
    void    GetActiveSnapshotPages(List<SnapshotPage>* plist);
    void    NextSnapshot(Snapshot* pnewSnapshot);
};

enum SnapshotState
{   
    SS_Active,
    SS_Captured,
    SS_Displaying,
    SS_Finalizing,
    SS_Total_States
};


//--------------------------------------------------------------------
// ***** Context Implementation

class Context
{    
    friend class Entry;
    friend class RenderNotify;
    friend class RTHandle;
    friend class EntryTable;
    friend struct RTHandle::HandleData;
public:

    typedef ContextImpl::Entry          Entry;
    typedef ContextImpl::EntryData      EntryData;
    typedef ContextImpl::EntryChange    EntryChange;
    typedef ContextImpl::ChangeBuffer   ChangeBuffer;
    typedef ContextImpl::RenderNotify   RenderNotify;


    // *** Context Public API

    Context(MemoryHeap* pheap);
    ~Context();

    // Creates a context entry node. T must be derived from Entry.
    template<class T>
    T*      CreateEntry()
    {
        EntryData* pdata = (EntryData*)
            SF_HEAP_ALLOC(pHeap, sizeof(typename T::NodeData), StatRender_NodeData_Mem);
        Construct<typename T::NodeData>(pdata);
        return (T*)createEntryHelper(pdata);
    }

    // Creates an entry with an initializing argument (or copy constructor).
    // T must be derived from Entry.
    template<class T, class S>
    T*      CreateEntry(const S& source)
    {
        EntryData* pdata = (EntryData*)
            SF_HEAP_ALLOC(pHeap, sizeof(typename T::NodeData), StatRender_NodeData_Mem);
        ConstructAlt<typename T::NodeData, S>(pdata, source);
        return (T*)createEntryHelper(pdata);
    }    

    // This flag is set if Capture was called at least once.
    bool    HasCapture() const { return CaptureCalled; }

    // Returns true if the current modifiable state of Context has changes,
    // this can be used to determine whether the Capture call is necessary.
    bool    HasChanges() const;

    // Notifies the context that a DrawableImage has been updated, and thus, HasChanges
    // should report true when next queried.
    void    SetDIChangesRequired() { DIChangesRequired = true; }

    // Captures the state of the render nodes so that further
    // modifications don't affect the displayed graph.
    bool    Capture();

    // Shutdown, called from Advance thread, begins the
    // shutdown procedure disables capture and will trigger
    // ShutdownRendering call on the rendering thread.
    // If 'waitFlag' it true, function will return only
    // after shutdown is complete; this is done automatically
    // in destructor.
    void    Shutdown(bool waitFlag);
    // Returns true once Shutdown() operation has finished, allowing
    // context to be destroyed without blocking in destructor.
    bool    IsShutdownComplete() const;

    // Propagates changed entries up the parent chains.
    // By default called automatically from within Capture.
    void    PropagateChangesUp();
    
    // GetFrameId returns id of the current frame, incremented by each Capture call.
    UInt64  GetFrameId() const { return SnapshotFrameIds[SS_Active]; }
    // GetFinalizedFrameId() returns the largest FrameId that was finalized.
    // Once a frame is finalized, its shared resources are known to be released.
    // When you release a TreeShape which has shape data references, these
    // references will persist until (GetFinalizedFrameId() >= recordedFrameId),
    // where recordedFrameId is the frame where TreeShape was destroyed.
    UInt64  GetFinalizedFrameId() const { return FinalizedFrameId; }

    // Forces update of cache nodes in order to use changed textures
    void    ForceUpdateImages();

    // Sets the thread id that is expected to call capture. An assert will trigger (in debug mode)
    // if a thread other than this thread calls capture, or otherwise reads/modifies the context data.
    // This only needs to be called explicitly if it is different than the thread that created the context.
    void SetCaptureThreadId(ThreadId captureThreadId);

    // Adds a notification object that will be called when Capture and NextCapture
    // events take place (possibly on different threads).
    void AddCaptureNotify(ContextCaptureNotify* notify);
    void RemoveCaptureNotify(ContextCaptureNotify* notify);

    MemoryHeap* GetHeap() const { return pHeap; }

    // *** Render Thread API

    
    enum CaptureMode
    {
        Capture_Immediate,
        Capture_OnceAFrame
    };
    // Captures the items from Display.
    // If mode is Capture_OnceAFrame, NextCapture will grab data only
    // first time within a frame.
    // Returns true on success, 'false' if context is being shut down.
    bool    NextCapture(RenderNotify* pnotify = 0,
                        CaptureMode mode = Capture_Immediate);

    // Called to once we are no longer rendering this context.
    void    ShutdownRendering(RenderNotify* pnotify = 0);    

private:
    Context* getThis()  { return this; }
    Lock*    getLock() const { return &pCaptureLock->LockObject; }

    void    endFrame()  { NextCaptureCalledInFrame = false; }

    // Accessor used by RenderNotify to determine is RenderingShutdown
    // needs to be called on context.
    bool     needRenderShutdown() const { return (pShutdownEvent != 0); }

    MemoryHeap*       pHeap;    
    ThreadId          CreateThreadId;
#ifdef SF_BUILD_DEBUG
    ThreadId          CaptureThreadId;  // Expected thread ID that will call Capture.
#endif
    EntryTable        Table;
    Ptr<ContextLock>  pCaptureLock;    
    List<ContextCaptureNotify> CaptureNotifyList;

    // Renderer that owns us. We need to finish with it before
    // being used with a new one.
    RenderNotify*   pRenderer;
    bool            MultiThreadedUse;
    bool            NextCaptureCalledInFrame;
    volatile bool   CaptureCalled;
    volatile bool   DIChangesRequired;
    // Set once Shutdown is initiated, can't be un-set.
    volatile bool   ShutdownRequested;    
    // If not null, RenderThread should set this even in ShutdownRendering.
    // Modified in lock, but can be polled through 
    Event* volatile pShutdownEvent;    
    // RenderNotify object keeps track of us through this list.
    RenderNotify::ContextNode RenderNode;

    List<RTHandle::HandleData> RTHandleList;    


    // Current snapshots for all stages. A single pointer is enough
    // for each slot because:
    //  - Consecutive Captured states are merged into one.
    //  - There can only be one Finalizing state at a time because (a) it is only generated
    //    by NextCapture() if Captured state exits and in that case Captured state
    //    is consumed; while (b) Additional Captured state can only be generated by
    //    Capture(), which also consumes any existing Finalizing state during its Lock.
    Snapshot*   pSnapshots[SS_Total_States];
    UInt64      SnapshotFrameIds[SS_Total_States];
    UInt64      FinalizedFrameId;

    // Helper function used to create & initialize nodes.
    Entry*      createEntryHelper(EntryData* pdata);

    // Part of NextCapture that requires a lock. Sets updateSnapshot to non-zero
    // if HandleChanges and destroy processing is necessary.
    bool        nextCapture_LockScope(Snapshot** updateSnapshot,
                                      RenderNotify* pnotify, CaptureMode mode);
    // Part of NextCapture that notifies renderer about changes; it generally should
    // be done outside of the lock since it can be expensive.
    void        nextCapture_NotifyChanges(Snapshot* displaySnaphot, RenderNotify *pnotify);


    void        handleFinalizingSnaphot();
    void        shutdownRendering_NoLock();

    // Destroys a snapshot from destructor.
    void        destroySnapshot(Snapshot* p);
    // Destroys entries of the given DestroyNodes list, clearing it.
    void        destroyNativeNodes(EntryList& destroyList);

    // Clears out RTHandle associated with an entry, causing the handle to be "null".    
    void        clearRTHandle(Entry* entry);
    void        clearRTHandleList();
};


// ***** Context inline Implementations

// These inlines are needed for public GetReadOnlyData/GetDisplayData access functions.
// The rest of inlines are hidden in source file.

const EntryData* Entry::GetReadOnlyDataBase() const
{    
    // If you hit this assert, it means that the thread responsible for the lifetime of the
    // context is not the one accessing its data. This function and those that call it should 
    // only be called from the thread that created the context.
    SF_DEBUG_ASSERT(getContext()->CaptureThreadId == GetCurrentThreadId(), 
        "Entry::GetReadOnlyDataBase() is called from wrong thread: must be the thread "
        "where Context was created.\nMake sure to synchronize and then use Movie.SetCaptureThread");
    //!AB: can't add ASSERT here, since this method can be called from another thread by DAPI.

    return EntryRef(this).GetData();
}
const EntryData* Entry::GetDisplayDataBase() const
{    
    return EntryRef(this).GetDisplayData();
}

EntryRef::EntryRef(const Entry* pentry)
{
    pPage = (EntryPage*)( ((UPInt)pentry) & ~(UPInt)(EntryPageAllocAlign-1) );
    Index = unsigned(pentry - &pPage->Entries[0]); // Pointer difference yields # of items.
    SF_ASSERT(pentry == pPage->Entries + Index);
}
EntryData* EntryRef::GetData() const
{
    return pPage->pSnapshotPage->pData[Index];
}
EntryData* EntryRef::GetCleanData() const
{   // Used in Capture finalize processing only for pointer matching.
    // In other places, pData should be clean already (or not be accessed).
    return pPage->pSnapshotPage->GetCleanData(Index);
}
EntryData* EntryRef::GetDisplayData() const
{
    return pPage->pDisplaySnapshotPage->GetCleanData(Index);
}



extern unsigned ConstructCopyCalls, DestroyCalls, CopyCalls;

// TBD: Make a template which does assignment!

template <class C, class B>
class ContextData_ImplMixin : public B
{
public:
    ContextData_ImplMixin(Context::EntryData::EntryType type = Context::EntryData::ET_Base)
        : B(type) { }
    ContextData_ImplMixin(NonlocalCloneArg<ContextData_ImplMixin<C, B> > src)
        : B(NonlocalCloneArg<B>(*src.pC)) { }

    virtual Context::EntryData* ConstructCopy(LinearHeap& heap) const
    {
        ConstructCopyCalls++;
        return ConstructAlt<C>(heap.Alloc(sizeof(C)), *(const C*)this);
    }
    virtual void    CopyTo(void *pdest) const
    {
        *(C*)pdest = *(const C*)this;
        CopyCalls++;
    }
    virtual void    Destroy()
    {
        DestroyCalls++;
        ((C*)this)->~C();
    }  
};

// Typecast-simplifying macro, to be used in classes derived from Context::Entry.
// These macros ensure that GetWritable/ReadOnly/DisplayData return the correct type instead of base class.
#define SF_RENDER_CONTEXT_ENTRY_INLINES(class) \
    inline class*       GetWritableData(unsigned cb) { return (class*)Entry::getWritableData(cb); }     \
    inline const class* GetReadOnlyData() const  { return (const class*)Entry::GetReadOnlyDataBase(); } \
    inline const class* GetDisplayData() const   { return (const class*)Entry::GetDisplayDataBase(); }


}}} // Scaleform::Render::ContextImpl

#endif
