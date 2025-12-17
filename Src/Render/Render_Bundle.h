/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Bundle.h
Content     :   Defines SortKey and BundleEntry classes, maintaining
                sorted Bundle lists for tree.
Created     :   September 23, 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Bundle_H
#define INC_SF_Render_Bundle_H

#include "Render_Containers.h"
#include "Render_Primitive.h"
#include "Render_Stats.h"
#include "Render_States.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_HeapNew.h"

#include <stdio.h>


namespace Scaleform { namespace Render {

/*  
    ***** Sorter Purpose

    The job of a Sorter is to generate and maintain a sorted list of Bundle objects that
    represent the rendering tree and can thus be used to display it more efficiently.
    To this end it uses a variety of objects:

     1) Bundle  - This is a group of one or more BundleEntry(s) of the same Key; bundle 
                  entries can typically be drawn together through batching. A list of
                  generated bundles for a TreeCacheNode can be traversed through a
                  BundleIterator; this is typically only needed for tree root.

     2) BundleEntry - A single element of a bundle, accessed by an array index. These
                      objects are maintained within leaf rendering tree nodes.
                      BundleEntry typically represents a single shape or a mesh, 
                      depending on tree implementation. Each BundleEntry also has
                      a Key that is used for sorting it.

     3) SortKey - A unit of Bundle sorting; consecutive BundleEntries that have the
                    same key are combined together into bundles.

   

    ***** Usage

    To use the sorter for an external rendering tree, the user needs to do the following:

     2) Implement Key-derived hashed sorting key. Place BundleEntry into each leaf tree node,
        such as a shape. BundleEntry constructor will require the Key, tree node that
        it is a part of, and SortManager.

     3) Derive base TreeCacheNode from TreeNodeMixin<>, which will inject sorter-required
        data into every tree node.
*/


// ***** Forward Declarations

class TreeCacheNode;
class TreeCacheRoot;
class Renderer2DImpl;

class BundleEntry;
class BundleIterator;
class BundleEntryRange;
class Bundle;



// SortKey is a class we use for sorting.
// Sorting key is a handle-type object used for sort type comparison,
// currently treated as pointer to RefCountBase<>, with low bits storing
// type. ThreadSafe ref-counting is used because this can hold MeshProvider.

// SortKey is a key used by sorter, which can take on multiple types.
// Sorting can use the following keys:
//  - PrimitiveFill - for simple meshes
//  - MeshShapeProvider - for complex meshes
// The following may also affect sorting
//  - 3D bit
//  - Effect chain (masks, filters, etc)

enum SortKeyFlags
{
    SKF_RangeStart          = 0x1000,   // The SKI is the beginning of a range.
    SKF_MatchNoOverlap      = 0x2000,   // The SKI can match with compatible SKIs, but not if they overlap.
    SKF_MatchSingleOverlap  = 0x4000,   // The SKI can match with compatible SKIs, if they are adjacent and overlapping.
    SKF_MatchNever          = 0x8000,   // The SKI will not match, even with compatible SKIs.
};

enum SortKeyType
{
    SortKey_None,
    SortKey_PrimitiveFill,
    SortKey_MeshProvider,
    SortKey_Text,
    // Masking
    SortKey_MaskStart,
    SortKey_MaskEnd,    // End or Pop.
    // Blend mode
    SortKey_BlendModeStart,
    SortKey_BlendModeEnd,
    // Filter mode
    SortKey_FilterStart,
    SortKey_FilterEnd,
    // ViewMatrix3D
    SortKey_ViewMatrix3DStart,
    SortKey_ViewMatrix3DEnd,
    // ProjectionMatrix3D
    SortKey_ProjectionMatrix3DStart,
    SortKey_ProjectionMatrix3DEnd,
    // UserData (setRenderString/Float)
    SortKey_UserDataStart,
    SortKey_UserDataEnd,
};

// Separate mask-type alias is declared for masks to enable
// a unique SortKey constructor to be used.
// NOTE: Must not change order
enum SortKeyMaskType
{
    SortKeyMask_Push,
    SortKeyMask_End,
    SortKeyMask_Pop,
    SortKeyMask_Item_Count
};

enum SortKeyTextType
{
    SortKeyText
};

// Transition result for GetRangeTransition() function for range-affecting
// keys such as masks. A transition range begins with a key marked with SKF_RangeStart
// flag and follows this pattern: {RangeStart -> Skip* -> Replace -> Skip* -> End}.
enum SortKeyTransition
{
    SKT_Skip,
    SKT_Replace,
    SKT_End
};

typedef void* SortKeyData;
class SortKey;

struct SortKeyInterface
{
    SortKeyType Type;
    unsigned    Flags;    

    virtual ~SortKeyInterface() {}

    virtual void AddRef(SortKeyData) { }
    virtual void Release(SortKeyData) { }
    virtual SortKeyTransition GetRangeTransition(SortKeyData, const SortKey&) { return SKT_Skip; }
    // UpdateBundleEntry is responsible for creating the bundle, if necessary,
    // and 
    virtual bool UpdateBundleEntry(SortKeyData, BundleEntry*, TreeCacheRoot*, Renderer2DImpl*, const BundleIterator&) { return false; }
    virtual void DrawBundleEntry(SortKeyData, BundleEntry*, Renderer2DImpl*) { }

    SortKeyInterface(SortKeyType type, unsigned flags = 0) : Type(type), Flags(flags) { }

    static SortKeyInterface NullInterface;
};

struct SortKeyInterface_RefCountImpl : public SortKeyInterface
{
    virtual void AddRef(SortKeyData p)  { if (p) ((RefCountImpl*)p)->AddRef(); }
    virtual void Release(SortKeyData p) { if (p) ((RefCountImpl*)p)->Release(); }
    
    SortKeyInterface_RefCountImpl(SortKeyType type, unsigned flags = 0)
     : SortKeyInterface(type, flags)
    { }
};


class SortKey
{
protected:
    SortKeyInterface* pImpl;
    SortKeyData       Data;

    void          addRef() const  { pImpl->AddRef(Data); }
    void          release() const { pImpl->Release(Data); }

    // Protected constructor for derived-class use.
    void initKey(SortKeyInterface* i, SortKeyData data)
    {        
        pImpl = i;
        Data  = data;
        i->AddRef(data);
    }

public:

    SortKey() : pImpl(&SortKeyInterface::NullInterface), Data(0) { }
    SortKey(SortKeyInterface* i, SortKeyData data) { initKey(i, data); }
    SortKey(const SortKey& src) : pImpl(src.pImpl), Data(src.Data) { addRef(); }
    ~SortKey() { release(); }

    // Key-type specific constructors (External)
    SortKey(PrimitiveFill* fill, bool is3D = false);
    SortKey(MeshProvider* fill, bool is3D = false);    
    SortKey(SortKeyMaskType maskType);
    SortKey(SortKeyType blendKeyType, BlendMode mode);
    SortKey(SortKeyTextType t, bool is3D);
    SortKey(SortKeyType filterKeyType, const FilterSet* filters);
    SortKey(SortKeyType viewKeyType, const Matrix3FRef* viewMatrix);
    SortKey(SortKeyType projKeyType, const Matrix4FRef* projMatrix);
    SortKey(SortKeyType projKeyType, UserDataState::Data* data);


    SortKeyType GetType() const { return pImpl->Type; }
    unsigned    GetFlags() const { return pImpl->Flags; }
    SortKeyData GetData() const { return Data; }

    PrimitiveFill* GetPrimitiveFill() const
    {
        SF_ASSERT(GetType() == SortKey_PrimitiveFill);
        return static_cast<PrimitiveFill*>(Data);
    }
    MeshProvider* GetMeshProvider() const
    {
        SF_ASSERT(GetType() == SortKey_MeshProvider);
        return static_cast<MeshProvider*>(Data);
    }
    
    SortKey& operator = (const SortKey& src)
    {
        src.addRef();
        release();
        pImpl = src.pImpl;
        Data  = src.Data;
        return *this;
    }

    bool operator == (const SortKey& other) const
    { return (pImpl == other.pImpl) && (Data == other.Data); }
    bool operator != (const SortKey& other) const
    { return ! operator == (other); }

    
    // It's possible to modify key under rare circumstances (Effect Update).
    // It is important to verify that this happens only when key-owner BundleEntry is not a
    // part of a Bundle with different key type.
    void SetData(SortKeyData data)
    {
        Data = data;
    }


    // *** Match functions
    
    // These functions are different for every key type    
    bool MatchNoOverlap(const SortKey& other)
    {
        if (GetFlags() & SKF_MatchNever)
            return false;
        return (Data == other.Data) && (pImpl == other.pImpl) && (GetFlags() & SKF_MatchNoOverlap);
    }
    bool MatchSingleItemOverlap(const SortKey& other)
    {
        if (GetFlags() & SKF_MatchNever)
            return false;
        return (Data == other.Data) && (pImpl == other.pImpl) && (GetFlags() & SKF_MatchSingleOverlap);
    }
    
    // *** Range-key matching APIs

    bool IsRangeStartKey() const { return (GetFlags() & SKF_RangeStart) != 0; }

    SortKeyTransition GetRangeTransition(const SortKey& other) const
    {
        return pImpl->GetRangeTransition(Data, other);        
    }

    // *** Rendering support

    bool UpdateBundleEntry(BundleEntry* e, TreeCacheRoot* tr, Renderer2DImpl* r, const BundleIterator& ibundles)
    { return pImpl->UpdateBundleEntry(Data, e, tr, r, ibundles); }
    void DrawBundleEntry(BundleEntry* e, Renderer2DImpl* r)
    { pImpl->DrawBundleEntry(Data, e, r); }
};




//--------------------------------------------------------------------
// **** BundleEntry
// 
// BundleEntry is a pattern-matching entity representing a single mesh,
// complex primitive or a rendering command; it is associated with a TreeCacheNode.
// Each BundleEntry has a Key that is used for sorting and batching. Entries
// with matching keys can be chained together though pChain pointers,
// forming a logical "Bundle" that can be drawn together as a unit. A bundle
// will often be turned into a single batch for rendering.
//  
// Separate BundleEntries are also linked to each other through pNextPattern
// pointer, creating a BundleEntryRange of items intended to be drawn
// independently one after another. pNextPattern are traversed through
// a BundleIterator object, which stops once it hits an explicit Last entry.

class BundleEntry
{
    friend class Bundle;
    friend class BundleIterator;
    friend class BundleEntryRange;
    friend class BundleEntryRangeMatcher;
private:
    // pNextPattern is hidden on purpose so as not to be modified explicitly.
    // Patterns are NOT terminated by null entry but rather by "last" item
    // tracked explicitly.
    //  - It is important not to null pNextPattern in last item since it
    //    may be a part of a higher-level chain.
    BundleEntry*  pNextPattern;
public:
    BundleEntry*  pChain;       // Null-terminated chain of identical-Key entries.
    UInt16        ChainHeight;
    UInt16        IndexHint;    // Index in pBundle; may be incorrect.
    SortKey       Key;
    Ptr<Bundle>   pBundle; //
    bool          Removed;

    inline void    SetBundle(Bundle* b, UPInt index);
    inline void    ClearBundle();

    // For now; could use type-based access 
    TreeCacheNode* pSourceNode;

    inline BundleEntry(TreeCacheNode* node, const SortKey& key);
    inline ~BundleEntry();

    // Accessors
    void     SetNextPattern(BundleEntry* nextPattern) { pNextPattern = nextPattern; }    
    bool     HasNextPattern() const                   { return pNextPattern != 0; }
    BundleEntry* GetNextPattern() const               { return pNextPattern; }


    bool UpdateBundleEntry(TreeCacheRoot* tr, Renderer2DImpl* r, const BundleIterator& ibundles);

    void DrawBundleEntry(Renderer2DImpl* r)
    {
        Key.DrawBundleEntry(this, r);
    }

private:
    // clearBundlePtr doesn't remove entry from Bundle array,
    // so its for Bundle-internal use only.
    inline void  clearBundlePtr();
};

// BundleInterator iterates over a list of BundleEntry objects typically represented
// by a BundleEntryRange. Each entry is typically a top of an entry chain for a tree level,
// and typically has an associated Bundle structure at the root level.
class BundleIterator
{
    BundleEntry* pFirst;
    BundleEntry* pLast;

public:
    BundleIterator() : pFirst(0), pLast(0) { }
    BundleIterator(BundleEntry* first, BundleEntry* last) : pFirst(first), pLast(last) { }
    BundleIterator(const BundleIterator& src) : pFirst(src.pFirst), pLast(src.pLast) { }

    BundleEntry* operator -> () const { return pFirst; }
    operator bool () const            { return (pFirst != 0); }
    operator BundleEntry* () const    { return pFirst; }

    BundleEntry* GetEntry() const     { return pFirst; }
    Bundle* GetBundle() const         { return pFirst->pBundle.GetPtr(); }

    void operator ++ (int)
    {
        SF_ASSERT(pFirst);
        if (pFirst == pLast)
            pFirst = pLast = 0;
        else
            pFirst = pFirst->pNextPattern;
    }
};


//--------------------------------------------------------------------

enum BundleRangeType
{
    Range_Empty   = 0,
    Range_Invalid = 0x80000000
};


// BundleEntryRange can be in either Empty or Invalid.
// - Invalid range is different from Empty in that it is not initialized,
//   so its child subtree must be built-up.

class BundleEntryRange
{
    friend class BundleEntryRangeMatcher;
    BundleEntry*  pFirst;
    BundleEntry*  pLast;
    UInt32        Length;    
public:

    BundleEntryRange(BundleRangeType rangeType)
        : pFirst(0), pLast(0), Length((UInt32)rangeType)
    { }

    BundleIterator GetIterator() const { return BundleIterator(pFirst, pLast); }

    void Clear(BundleRangeType rangeType)
    {
        pFirst = pLast = 0;
        Length = (UInt32)rangeType;
    }

    void Init(BundleEntry* first, BundleEntry* last, UInt32 length)
    {
        pFirst = first;
        pLast = last;
        Length = length;
    }

    // Invalid range is also empty.
    bool IsEmpty() const { return pFirst == 0; }
    bool IsValid() const { return Length != Range_Invalid; }    

    unsigned     GetLength() const  { return (unsigned)(Length & ~Range_Invalid); }
    BundleEntry* GetFirst() const   { return pFirst; }
    BundleEntry* GetLast() const    { return pLast; }
    SortKey&     GetFirstKey() const { return pFirst->Key; }

    void SetToEntry(BundleEntry* e)
    {
        pFirst = pLast = e;
        Length = 1;
    }

    void VerifyValidChildDepth(); 

    // Removes all chain entries connected at higher depth and adds
    // item to range.
    void StripChainsByDepth(unsigned topDepth);
    // Strips all chains unconditionally; useful for leaf nodes.
    void StripChains();
    

    void Append(BundleEntryRange& next)
    {        
        if (next.IsEmpty())
            return;
        SF_ASSERT(IsValid());

        if (!IsEmpty())
            pLast->pNextPattern = next.pFirst;
        else
            pFirst = next.pFirst;
        pLast = next.pLast;
        Length += next.Length;
    }

};

// BundleEntryRangeMatcher implements a simple cache of last entries,
// which dramatically improves Match performance as it avoids repetitive chain
// traversal. For extreme example, frogger.swf (Ctrl+G) spent 30% of its time
// in match; this imporved its perf 673 -> 890 fps.
// TBD: For slightly better improvement we could keep pLastChain in entries, which might
// also optimize StripChainsByDepth; however, that would involve significant debugging...

enum {
    Bundle_MergeLengthLimit = 8
};

class BundleEntryRangeMatcher : public BundleEntryRange
{    
    mutable BundleEntry* pLastEntries[Bundle_MergeLengthLimit];
    mutable UPInt        LastEntryCount;

    inline BundleEntry* getLastEntry(UPInt index) const;
    inline void         setLastEntry(UPInt index, BundleEntry* entry) const;

public:
    BundleEntryRangeMatcher(BundleRangeType rangeType)
        : BundleEntryRange(rangeType), LastEntryCount(0) { }

    void operator = (const BundleEntryRange& other)
    {
        BundleEntryRange::operator = (other);
        LastEntryCount = 0;
    }

    // Determined if the other pattern range is a subset of us.
    // returns 'true' if it is.
    // Match uses a "key stack" internally.
    bool Match(BundleEntryRange& other, unsigned *mergeDepth = 0) const;

    void MergeMatchingChains(BundleEntryRange& other, unsigned mergeDepth)
    {
        Match(other, &mergeDepth);
    }
};


//--------------------------------------------------------------------

// Bundle is a base class for Render::PrimitiveBundle and represents an array
// of BundleEntry(s).
class Bundle : public RefCountBaseNTS<Bundle, StatRender_RenderPipeline_Mem>
{   
protected:    
    ArrayLH<BundleEntry*> Entries;
    BundleEntry *pTop;
    bool  NeedUpdate;

public:
    unsigned FrameId;

    Bundle(BundleEntry* top) : NeedUpdate(true)
    {
        FrameId =0;
        UpdateChain(top);
        NeedUpdate = true;
    }

    ~Bundle()
    {
    }

    void     SetNeedsUpdate()    { NeedUpdate = true; }
    bool     NeedsUpdate() const { return NeedUpdate; }

    unsigned GetEntryCount() const { return (unsigned)Entries.GetSize(); }

    void UpdateChain(BundleEntry *newTop);
    
    // Insertion and removal are overridable to allow extra operations,
    // such as mesh updates. These methods are called during child insert/remove sorting.
    virtual void InsertEntry(UPInt index, BundleEntry* shape);
    virtual void RemoveEntries(UPInt index, UPInt count);
    virtual void UpdateMesh(BundleEntry* entry);

    void RemoveEntry(BundleEntry* entry);
    bool findEntryIndex(UPInt *pindex, BundleEntry* entry);
};

inline BundleEntry::BundleEntry(TreeCacheNode* node, const SortKey& key)
    : pNextPattern(0), pChain(0), ChainHeight(0), IndexHint(0),
      Key(key), pSourceNode(node)
{
    Removed = false;
}

inline BundleEntry::~BundleEntry()
{
    if (pBundle)
    {
        Ptr<Bundle> oldBundle = pBundle;
        pBundle->RemoveEntry(this);
    }
}

void BundleEntry::SetBundle(Bundle* b, UPInt indexHint)
{
    if (pBundle && pBundle != b)
    {
        Ptr<Bundle> oldBundle = pBundle;
        oldBundle->RemoveEntry(this);
    }
    pBundle    = b;
    IndexHint  = (UInt16)indexHint;
}

void BundleEntry::ClearBundle()
{
    if (pBundle)
    {
        Ptr<Bundle> oldBundle = pBundle;
        oldBundle->RemoveEntry(this);
    }
    clearBundlePtr();
}

inline void  BundleEntry::clearBundlePtr()
{
    pBundle.Clear(); IndexHint = 0;
}

}} // Scaleform::Render

#endif
