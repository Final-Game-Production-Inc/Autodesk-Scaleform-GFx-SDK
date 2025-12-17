/**************************************************************************

PublicHeader:   Render
Filename    :   Render_MeshKey.h
Created     :   2005-2010
Authors     :   Maxim Shemanarev, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef SF_Render_MeshKey_H
#define SF_Render_MeshKey_H

#include "Kernel/SF_List.h"
#include "Render_Primitive.h"

namespace Scaleform { namespace Render {

struct ToleranceParams;

// Implemented here:
class MeshKey;
class MeshKeySet;
class MeshKeyManager;
class MeshProvider_KeySupport;


//------------------------------------------------------------------------
// ***** MeshKey

// MeshKey represents a single orientation of a Shape that can be
// rendered with a single mesh. MeshProviders can have multiple Mesh and
// MeshKey objects used for different scales. MeshKey is created by a call
// to MeshKeyManager::CreateMatchingKey; a single MeshKey can be shared
// my multiple tree nodes.
//
// Although MeshKey uses reference counting, it is a hint because
// meshes can survive after past their UseCount hits 0; they live as long
// as their Mesh data is still in cache.
// MeshKey and its cache data is destroyed when MeshProvider object dies;
// this is handled through thread-safe KillList in MeshKeyManager.


class MeshKey : public ListNode<MeshKey>
{
public:
    enum { MaxKeySize = 20 };

    enum KeyFlags 
    {
        KF_Fill          = 0x0001,
        KF_Stroke        = 0x0002,
        KF_StrokeHinted  = 0x0003,
        //                 0x0004, // Reserved
        //                 0x0005,
        //                 0x0006,
        //                 0x0007,
        KF_KeyTypeMask   = 0x0007,

        KF_Scale9Grid    = 0x0010,
        KF_NormalStroke  = 0x0020, // In case of normal stroke scale, we can optimize uniform scaling
        KF_EdgeAA        = 0x0040,
        KF_Mask          = 0x0080,
        KF_GradientMorph = 0x0100,

        KF_Degenerate    = 0x8000  // Degenerate matrix; any key matches
    };

    MeshKey() : pKeySet(0), Size(0), Flags(0), UseCount(1) {}

    static unsigned GetKeySize(unsigned flags);
    static bool     CalcMatrixKey(const Matrix2F& m, float* key, Matrix2F* m2);
    static bool     CalcMatrixKey_NonOpt(const Matrix2F& m, float* key, Matrix2F* m2);

    bool Match(unsigned layer, unsigned flags,
               const float* keyData, const ToleranceParams& cfg) const;

    void AddRef()  { ++UseCount; }
    void Release();
    bool NotInUse() const { return UseCount == 0; }

    MeshKeySet* GetKeySet() const { return pKeySet; }

public:
    MeshKeySet* pKeySet;
    Ptr<MeshBase> pMesh;
    UInt16      Size;            // Size (number of values)
    UInt16      Flags;           // EdgeAA, StrokeType, etc
    unsigned    UseCount;        // How many TreeCacheShapeLayers use this key
    float       Data[1];
};


//------------------------------------------------------------------------
// Handle maintained in MeshProvider to
//  - Find MeshKeys efficiently based on provider pointer
//  - Inform cache system when provider is destroyed
//    (needed to free up cache space and ensure there are no
//     same-address pointer matches in case of re-allocation).
//
// This class is Partially thread safe as follows:
//  - Its constructor and destructor can be called on any thread.
//    Destructor notifies MeshKeyManager in thread-safe manager.
// Besides destruction, this object is only accessed on the Render Thread:
//  - pKeySet can be read without a lock by RenderThread only,
//    it is always modified during MeshKeyManagerLock.
//  - Render thread will call releaseCache_NoNotify() when it
//    swaps out the associated MeshKeySet.

class MeshKeySetHandle
{
    friend class MeshKeyManager;
    friend class MeshKeySet;
    friend class MeshProvider_KeySupport;

    AtomicPtr<MeshKeyManager> pManager;
    MeshKeySet* volatile      pKeySet;
    void releaseCache();
    void releaseCache_NoNotify();

public:
    MeshKeySetHandle() : pKeySet(0)
    { }
    ~MeshKeySetHandle()
    { releaseCache(); }

    inline bool IsEmpty() const;
    inline bool HasKeySet() const;
};

// MeshProvider_KeySupport injects MeshKey support into MeshProvider.
class MeshProvider_KeySupport : public MeshProvider_RCImpl
{
    friend class MeshKeyManager;
    friend class MeshKeySet;

    MeshKeySetHandle hKeySet;
protected:
    inline void releaseKeySet();
public:
    MeshProvider_KeySupport() { }
    ~MeshProvider_KeySupport() { }

    bool IsEmpty() const { return hKeySet.IsEmpty(); }
    bool HasKeySet() const { return hKeySet.HasKeySet(); }
};


//------------------------------------------------------------------------
// ***** MeshKeySet

// MeshKeySet represents a group of MeshKeys associated with one MeshProvider.
// MeshKeySets are created and tracked internally within MeshKeyManager,
// its lifetime is synchronized across threads.

class MeshKeySet : public ListNode<MeshKeySet>, public MeshProvider,
                   public NewOverrideBase<StatRender_Mem>
{
    friend class MeshKeyManager;
public:

    inline MeshKeySet(MeshKeyManager* manager, MeshProvider_KeySupport* provider);
    inline ~MeshKeySet();

    MeshKey* CreateKey(const float* keyData, unsigned flags);
    void     DestroyKey(MeshKey* key);

    // Mesh key should only be accessed by the rendering thread,
    // which is responsible for clearing this list when it is shut down.
    // Since TreeCacheNode objects are always destroyed before their main nodes,
    // the simplest way to enforce this is to use a counter.
    void     DestroyAllKeys();

    virtual void    OnEvict(MeshBase *mesh);


    // *** MeshProvider Delegation

    // These MeshProvider functions delegate to the original object; they should
    // not be called when pDelegate is null since that would indicate mesh
    // generation on a swapped-out provider object.

    virtual bool    GetData(MeshBase *mesh, VertexOutput* out, unsigned meshGenFlags)
    { return pDelegate->GetData(mesh, out, meshGenFlags); }
    virtual RectF   GetIdentityBounds() const
    { return pDelegate->GetIdentityBounds(); }
    virtual RectF   GetBounds(const Matrix2F& m) const
    { return pDelegate->GetBounds(m); }

    virtual RectF   GetCorrectBounds(const Matrix2F& m, float morphRatio, 
                                     StrokeGenerator* gen, const ToleranceParams* tol) const
    { return pDelegate->GetCorrectBounds(m, morphRatio, gen, tol); }

    virtual bool    HitTestShape(const Matrix2F& m, float x, float y, float morphRatio,
                                 StrokeGenerator* gen, const ToleranceParams* tol) const
    { return pDelegate->HitTestShape(m, x, y, morphRatio, gen, tol); }

    virtual unsigned    GetLayerCount() const { return pDelegate->GetLayerCount(); }
    
    virtual unsigned    GetFillCount(unsigned layer, unsigned meshGenFlags) const
    { return pDelegate->GetFillCount(layer, meshGenFlags); }

    virtual void        GetFillData(FillData* data, unsigned layer,
                                    unsigned fillIndex, unsigned meshGenFlags)
    { pDelegate->GetFillData(data, layer, fillIndex, meshGenFlags); }

    virtual void        GetFillMatrix(MeshBase *mesh, Matrix2F* matrix, unsigned layer,
                                      unsigned fillIndex, unsigned meshGenFlags)
    { pDelegate->GetFillMatrix(mesh, matrix, layer, fillIndex, meshGenFlags); }

    virtual bool IsValid() const { return (pDelegate != 0); }

    bool IsEmpty() const { return Meshes.IsEmpty(); }

private:

    Ptr<MeshKeyManager>       pManager;
    MeshProvider_KeySupport*  pDelegate; // Only modified during lock.
    List<MeshKey>             Meshes;
    
    MeshKey* findMatchingKey(unsigned layer, unsigned flags,
                             const float* keyData, const ToleranceParams& cfg);

    void    releaseDelegate_RenderThread();
};

inline bool MeshKeySetHandle::IsEmpty() const { return pKeySet == 0 || pKeySet->IsEmpty(); }
inline bool MeshKeySetHandle::HasKeySet() const { return pKeySet != 0; }

inline void MeshProvider_KeySupport::releaseKeySet()
{
    if (HasKeySet())
        hKeySet.releaseCache();
}


//------------------------------------------------------------------------
// ***** MeshKeyManager

// MeshKeyManager creates and manages lifetime of MeshKey objects,
// a single object of this class is created by Renderer2DImpl.
//
// Individual MeshKey objects are created by CreateMatchingKey call.
// 
// MeshKeyManager object is always created on global heap, so as
// to allow its internal Lock to survive until it is released in
// associated MeshProviders (synchronization exchange protocol
// in MeshKeySetHandle allows MeshKeyManager to be released on any thread).

class MeshKeyManager : public RefCountBase<MeshKeyManager, StatRender_Mem>
{
    friend class MeshKeySet;
    friend class MeshKeySetHandle;
    friend class Renderer2DImpl;
public:

    MeshKeyManager(MemoryHeap* renderHeap) : pRenderHeap(renderHeap) { }
    ~MeshKeyManager()
    {
        DestroyAllKeys();
    }

    // Cleans up meshes for destroyed MeshProvider objects.
    // Should be called every frame.
    void ProcessKillList();

    // Destroys all of the keys; useful when shutting down renderer.
    // Should be called only after all MeshKey references were released,
    // otherwise it will assert internally.    
    void DestroyAllKeys();

    // Find and/or creates a MeshKey reference with incremented use count.
    MeshKey* CreateMatchingKey(MeshProvider_KeySupport* provider,
                               unsigned layer, unsigned flags,
                               const float* keyData, const ToleranceParams& cfg);

    MeshKey* CreateMatchingKey(MeshKeySet *keySet,
                               unsigned layer, unsigned flags,
                               const float* keyData, const ToleranceParams& cfg);
private:

    enum KeySetListType
    {
        KeySet_LiveList,
        KeySet_KillList,
        KeySet_List_Count
    };

    Lock              KeySetLock;
    List<MeshKeySet>  KeySets[KeySet_List_Count];    
    // Heap used for MeshKey objects and other allocation; different from this heap
    // which is global for lock lifetime.
    MemoryHeap*       pRenderHeap;

    Lock* getLock() { return &KeySetLock; }

    // Called from ~MeshKeySetHandle to notify render thread
    // that Provider is being destroyed.
    void  providerLost(MeshKeySetHandle& handle);
    
    void  destroyKeySet(MeshKeySet* keySet);
    void  destroyKeySetList_NTS(KeySetListType type);
};

inline MeshKeySet::MeshKeySet(MeshKeyManager* manager, MeshProvider_KeySupport* provider)
           : pManager(manager), pDelegate(provider)
{
}

inline MeshKeySet::~MeshKeySet()
{
    DestroyAllKeys();
}

}} // Scaleform::Render

#endif

