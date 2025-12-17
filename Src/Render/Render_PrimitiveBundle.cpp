/**************************************************************************

Filename    :   Render_PrimitiveBundle.cpp
Content     :   PrimitiveBundle classes, SortKey and sorting support
Created     :
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_PrimitiveBundle.h"
#include "Renderer2DImpl.h" // for GetHAL() inline
#include "Render_Queue.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {

// ***** DrawableBundle
//
HAL* DrawableBundle::GetHAL() const
{
    return pRenderer2D->GetHAL();
}

// ***** PrimitiveBundle

// Insertions & removals should manipulate the Primitive and Matrices
PrimitiveBundle::PrimitiveBundle(TreeCacheRoot *proot, const SortKey& key,
                                 Renderer2DImpl* pr2d)
: DrawableBundle(proot, pr2d), Prim(GetHAL(), key.GetPrimitiveFill())
{    
}

void    PrimitiveBundle::CheckMatrixConsistency()
{
    unsigned count = Prim.GetMeshCount();

    for (unsigned i = 0; i< count; i++)
    {
        // Compute parent matrix.
        TreeCacheMeshBase* pshape = static_cast<TreeCacheMeshBase*>(Entries[i]->pSourceNode);
        Matrix             m(pshape->GetMatrixSafe());
        TreeCacheNode*     p = pshape->GetParent();

        while(p)
        {
            m.Append(p->GetNodeData()->M2D());
            p = p->GetParent();
        }        

        const Matrix &m2 = Prim.GetMatrix(i).GetMatrix2D();
        if (m2 != m)
            SF_ASSERT(0);
    }
}

void PrimitiveBundle::InsertEntry(UPInt index, BundleEntry* entry)
{
    DrawableBundle::InsertEntry(index, entry);
    TreeCacheMeshBase* source = (TreeCacheMeshBase*)entry->pSourceNode;
    SF_ASSERT(!source->M.IsNull()); 
    Prim.Insert((unsigned)index, (Mesh*)source->GetMesh(), source->M);
}

void PrimitiveBundle::RemoveEntries(UPInt index, UPInt count)
{
    Prim.Remove((unsigned)index, (unsigned)count);
    DrawableBundle::RemoveEntries(index, count);
}

void PrimitiveBundle::UpdateMesh(BundleEntry* entry)
{
    UPInt index;
    if (findEntryIndex(&index, entry))
    {
        TreeCacheMeshBase* source = (TreeCacheMeshBase*)entry->pSourceNode;
        Prim.SetMesh((unsigned)index, (Mesh*)source->GetMesh());
    }
}


//--------------------------------------------------------------------
// ***** Complex Primitive Bundle

ComplexPrimitiveBundle::ComplexPrimitiveBundle() : Bundle(0)
{
}
ComplexPrimitiveBundle::~ComplexPrimitiveBundle()
{
}

void ComplexPrimitiveBundle::Draw(HAL* hal)
{   
    // We need to emit every ComplexMesh as a separate draw call, since it is
    // a unit of caching that may need to be Prepared and emitted separately.
    // To do this, we count the numbed of consecutive meshes and pass a start
    // index for each new mesh start.

    UPInt count = Instances.GetSize();
    for (UPInt i = 0; i < count; i += countConsecutiveMeshesAtIndex(i))
        hal->Draw(this, (void*)(i)); // Pass index; count will be re-computed.
}


void ComplexPrimitiveBundle::InsertEntry(UPInt index, BundleEntry* entry)
{
    Bundle::InsertEntry(index, entry);
    
    TreeCacheMeshBase* source = (TreeCacheMeshBase*)entry->pSourceNode;
    ComplexMesh *      complexMesh = (ComplexMesh*)source->GetMesh();
    SF_ASSERT(!source->M.IsNull());

    InstanceEntry e = { source->M, complexMesh };
    Instances.InsertAt(index, e);   
}

void ComplexPrimitiveBundle::RemoveEntries(UPInt index, UPInt count)
{
    Instances.RemoveMultipleAt(index, count);
    Bundle::RemoveEntries(index, count);
}

void ComplexPrimitiveBundle::UpdateMesh(BundleEntry* entry)
{
    UPInt index;
    if (findEntryIndex(&index, entry))
    {
        TreeCacheMeshBase* source = (TreeCacheMeshBase*)entry->pSourceNode;
        Instances[index].pMesh = (ComplexMesh*)source->GetMesh();
    }
}


RenderQueueItem::QIPrepareResult
ComplexPrimitiveBundle::Prepare(RenderQueueItem& item,
                                RenderQueueProcessor& qp, bool waitForCache)
{
    if ( qp.GetQueuePrepareFilter() != RenderQueueProcessor::QPF_All )
        return RenderQueueItem::QIP_Done;

    // Upload into cache if not there already.
    UPInt        index = (UPInt)item.GetData();
    ComplexMesh* mesh = Instances[index].pMesh;

    bool prepareResult = qp.GetMeshCache()->PrepareComplexMesh(mesh, waitForCache);

    // We use PrepareItemBuffer::IsProcessing(this) state as a flag, setting it only if 
    // PrepareComplexPrimitive failed initially. In this case, we want to return QIP_NeedCache
    // once, which would force any items in render queue before us to be flushed,
    // freeing up cache for another Prepare attempt.
    // This means that our logic is as follows:
    //  - If we succeed the first time, just emit Done.
    //  - When we fail the first time, we set StartProcessing(this), causing a flush.
    //  - When we fail the second time, clear IsProcessing and return Done. In this
    //    case Emit will simply skip us if there is no MeshBatchData.

    if (prepareResult || qp.GetPrepareItemBuffer()->IsProcessing(&item))
    {
        qp.GetPrepareItemBuffer()->StartProcessing(0);
        return RenderQueueItem::QIP_Done;
    }

    qp.GetPrepareItemBuffer()->StartProcessing(&item);
    return RenderQueueItem::QIP_NeedCache;
}

void ComplexPrimitiveBundle::EmitToHAL(RenderQueueItem& item, RenderQueueProcessor& qp)
{
    if ( qp.GetQueueEmitFilter() != RenderQueueProcessor::QPF_All )
        return;

    // Execute rendering command on HAL...
    UPInt        index = (UPInt)item.GetData();
    ComplexMesh* mesh = Instances[index].pMesh;

    if (!qp.GetPrepareItemBuffer()->IsProcessing(&item) && mesh)
    {
        // Re-count the number of instances what we combined,
        // this is the same loop as was used to determine the index.
        UPInt  instanceCount = countConsecutiveMeshesAtIndex(index);
        StrideArray<HMatrix> matrices(&Instances[index].M, instanceCount,
                                      sizeof(InstanceEntry));
        qp.GetHAL()->DrawProcessedComplexMeshes(mesh, matrices);
    }
}


//--------------------------------------------------------------------
// ***** MaskBundle

// Insertions & removals should manipulate the Primitive and Matrices
MaskBundle::MaskBundle()
 : Bundle(0), Prim()
{
}

void MaskBundle::InsertEntry(UPInt index, BundleEntry* entry)
{
    Bundle::InsertEntry(index, entry);
    // Obtains "Mask Clear" matrix rectangle.
    MaskEffect* maskEffect = entry->pSourceNode->Effects.GetEffectOfType<MaskEffect, State_MaskNode>();
    SF_ASSERT(maskEffect);
    Prim.Insert(index, maskEffect->GetBoundsMatrix());
}

void MaskBundle::RemoveEntries(UPInt index, UPInt count)
{
    Prim.Remove((unsigned)index, (unsigned)count);
    Bundle::RemoveEntries(index, count);
}

//--------------------------------------------------------------------
// ***** FilterBundle

// Insertions & removals should manipulate the Primitive and Matrices
FilterBundle::FilterBundle(FilterSet* filters, bool maskPresent)
 : Bundle(0), Prim(filters, maskPresent)
{
}

void FilterBundle::InsertEntry(UPInt index, BundleEntry* entry)
{
    Bundle::InsertEntry(index, entry);
    FilterEffect* filterEffect = entry->pSourceNode->Effects.GetEffectOfType<FilterEffect,State_Filter>();
    SF_ASSERT(filterEffect);
    Prim.Insert(index, filterEffect->GetBoundsMatrix());
}

void FilterBundle::RemoveEntries(UPInt index, UPInt count)
{
    Prim.Remove((unsigned)index, (unsigned)count);
    Bundle::RemoveEntries(index, count);
}

//--------------------------------------------------------------------
// ***** BlendModeBundle

// Insertions & removals should manipulate the Primitive and Matrices
BlendModeBundle::BlendModeBundle(BlendMode mode, bool maskPresent)
 : Bundle(0), Prim(mode, maskPresent)
{
}

void BlendModeBundle::InsertEntry(UPInt index, BundleEntry* entry)
{
    Bundle::InsertEntry(index, entry);
    BlendModeEffect* blendEffect = entry->pSourceNode->Effects.GetEffectOfType<BlendModeEffect, State_BlendMode>();
    SF_ASSERT(blendEffect);
    Prim.Insert(index, blendEffect->GetBoundsMatrix());
}

void BlendModeBundle::RemoveEntries(UPInt index, UPInt count)
{
    Prim.Remove((unsigned)index, (unsigned)count);
    Bundle::RemoveEntries(index, count);
}

//--------------------------------------------------------------------
// ***** ViewMatrix3DBundle

// Insertions & removals should manipulate the Primitive and Matrices
ViewMatrix3DBundle::ViewMatrix3DBundle(Matrix3FRef* pvm)
: Bundle(0), Prim()
{
    if (pvm)
        Prim.SetViewMatrix3D(pvm->M);
}

//--------------------------------------------------------------------
// ***** ProjectionMatrix3DBundle

// Insertions & removals should manipulate the Primitive and Matrices
ProjectionMatrix3DBundle::ProjectionMatrix3DBundle(Matrix4FRef *ppm)
: Bundle(0), Prim()
{
    if (ppm)
        Prim.SetProjectionMatrix3D(ppm->M);
}

//--------------------------------------------------------------------
// ***** UserDataBundle
UserDataBundle::UserDataBundle(UserDataState::Data* data) :
    Bundle(0), Prim(data)
{

}

//--------------------------------------------------------------------
// ***** SortKey for Primitive/ComplexPrimitive implementation

class SKI_Primitive : public SortKeyInterface_RefCountImpl
{
public:
    SKI_Primitive()
        : SortKeyInterface_RefCountImpl(SortKey_PrimitiveFill, SKF_MatchNoOverlap|SKF_MatchSingleOverlap)
    { }

    static SKI_Primitive Instance;
    static SKI_Primitive Instance3D;

    virtual bool UpdateBundleEntry(SortKeyData d, BundleEntry* p,
                                   TreeCacheRoot* tr, Renderer2DImpl* r,
                                   const BundleIterator&)
    {
        if (!p->pBundle)
        {
            SortKey ourKey((PrimitiveFill*)d);
            Ptr<DrawableBundle> bundle = *SF_HEAP_AUTO_NEW(tr) PrimitiveBundle(tr, ourKey, r);
            p->SetBundle(bundle, 0);
        }
        return p->pBundle.GetPtr() != 0;
    }

    virtual void DrawBundleEntry(SortKeyData, BundleEntry* p, Renderer2DImpl*)
    {
        SF_AMP_SCOPE_RENDER_TIMER("SKI_Primitive::DrawBundleEntry", Amp_Profile_Level_High);
        if (p->pBundle)
            ((PrimitiveBundle*)p->pBundle.GetPtr())->Draw();
    }
};

class SKI_ComplexPrimitive : public SortKeyInterface
{
public: 
    SKI_ComplexPrimitive()
        : SortKeyInterface(SortKey_MeshProvider, SKF_MatchNoOverlap)
    { }

    static SKI_ComplexPrimitive Instance;
    static SKI_ComplexPrimitive Instance3D;

    // Use custom Ref-Counting since MeshProvider doesn't necessarily
    // start with RefCountImpl* base.
    virtual void AddRef(SortKeyData p)  { if (p) ((MeshProvider*)p)->AddRef(); }
    virtual void Release(SortKeyData p) { if (p) ((MeshProvider*)p)->Release(); }

    virtual bool UpdateBundleEntry(SortKeyData, BundleEntry* p,
                                   TreeCacheRoot* tr, Renderer2DImpl*,
                                   const BundleIterator&)
    {
        if (!p->pBundle)
        {
            Ptr<ComplexPrimitiveBundle> bundle = 
                *SF_HEAP_AUTO_NEW(tr) ComplexPrimitiveBundle();
            p->SetBundle(bundle, 0);
        }
        return p->pBundle.GetPtr() != 0;
    }

    virtual void DrawBundleEntry(SortKeyData, BundleEntry* p, Renderer2DImpl* r)
    {
        SF_AMP_SCOPE_RENDER_TIMER("SKI_ComplexPrimitive::DrawBundleEntry", Amp_Profile_Level_High);
        if (p->pBundle)
        {
            ComplexPrimitiveBundle* bundle = (ComplexPrimitiveBundle*)p->pBundle.GetPtr();
            bundle->Draw(r->GetHAL());
        }
    }
};

SKI_Primitive          SKI_Primitive::Instance;
SKI_Primitive          SKI_Primitive::Instance3D;
SKI_ComplexPrimitive   SKI_ComplexPrimitive::Instance;
SKI_ComplexPrimitive   SKI_ComplexPrimitive::Instance3D;

SortKey::SortKey(PrimitiveFill* fill, bool is3D)
{
    if ( !is3D )
        initKey(&SKI_Primitive::Instance, fill);
    else
        initKey(&SKI_Primitive::Instance3D, fill);
}

SortKey::SortKey(MeshProvider* fill, bool is3D)
{
    if ( !is3D )
        initKey(&SKI_ComplexPrimitive::Instance, fill);
    else
        initKey(&SKI_ComplexPrimitive::Instance3D, fill);
}

//--------------------------------------------------------------------
// ***** SortKey interfaces and constructors for Masks

class SKI_MaskStart : public SortKeyInterface
{
public:
    SKI_MaskStart(SortKeyType type, unsigned flags) : SortKeyInterface(type, flags)
    { }

    static SKI_MaskStart Combinable_Instance;

    virtual bool UpdateBundleEntry(SortKeyData, BundleEntry* p,
                                   TreeCacheRoot* tr, Renderer2DImpl*,
                                   const BundleIterator&)
    {
        if (!p->pBundle)
        {
            Ptr<MaskBundle> bundle = *SF_HEAP_AUTO_NEW(tr) MaskBundle();
            p->SetBundle(bundle, 0);
        }
        return p->pBundle.GetPtr() != 0;
    }

    virtual SortKeyTransition GetRangeTransition(SortKeyData, const SortKey& other)
    {
        if (other.GetType() == SortKey_MaskEnd)
        {
            SF_ASSERT(SortKeyMask_End == (UPInt)other.GetData());
            return SKT_Replace;
        }
        return SKT_Skip;
    }

    virtual void DrawBundleEntry(SortKeyData, BundleEntry* p, Renderer2DImpl* r)
    {
        SF_AMP_SCOPE_RENDER_TIMER("SKI_MaskStart::DrawBundleEntry", Amp_Profile_Level_High);
        if (p->pBundle)
        {
            MaskBundle *bundle = (MaskBundle*)p->pBundle.GetPtr();
            bundle->Draw(r->GetHAL());
        }
    }
};


class SKI_MaskEnd : public SortKeyInterface
{
public:
    SKI_MaskEnd(SortKeyType type, unsigned flags) : SortKeyInterface(type, flags)
    { }

    class RQII_EndMask : public RenderQueueItem::Interface
    {
    public:
        virtual void EmitToHAL(RenderQueueItem& item, RenderQueueProcessor& qp)
        {
            if ( qp.GetQueueEmitFilter() != RenderQueueProcessor::QPF_All )
                return;

            HAL*      hal = qp.GetHAL();
            SortKeyMaskType maskType = (SortKeyMaskType)(UPInt)item.GetData();
            if (maskType == SortKeyMask_End)
                hal->EndMaskSubmit();
            else if (maskType == SortKeyMask_Pop)
                hal->PopMask();
        }        
    };

    static RQII_EndMask RQII_Instance;
    static SKI_MaskEnd  Instance;

    virtual SortKeyTransition GetRangeTransition(SortKeyData data, const SortKey& other)
    {
        if (other.GetType() == SortKey_MaskEnd)
        {
            SF_ASSERT(SortKeyMask_End == (UPInt)data);
            SF_ASSERT(SortKeyMask_Pop == (UPInt)other.GetData());
            return SKT_End;
        }
        SF_UNUSED(data);
        return SKT_Skip;
    }

    virtual void DrawBundleEntry(SortKeyData data, BundleEntry*, Renderer2DImpl* r2d)
    {
        SF_AMP_SCOPE_RENDER_TIMER("SKI_MaskEnd::DrawBundleEntry", Amp_Profile_Level_High);
        RenderQueueItem rqi(&RQII_Instance, data);
        r2d->GetHAL()->Draw(rqi);
    }
};

// Static SortKeyInterface instances for masks.
SKI_MaskStart SKI_MaskStart::Combinable_Instance(SortKey_MaskStart,
                                                SKF_MatchNoOverlap|SKF_RangeStart);
SKI_MaskEnd   SKI_MaskEnd::Instance(SortKey_MaskEnd, SKF_MatchNoOverlap);

SKI_MaskEnd::RQII_EndMask SKI_MaskEnd::RQII_Instance;

static SortKeyInterface* SortKeyMaskInterfaces[SortKeyMask_Item_Count] =
{
    &SKI_MaskStart::Combinable_Instance,
    &SKI_MaskEnd::Instance,
    &SKI_MaskEnd::Instance
};

SortKey::SortKey(SortKeyMaskType maskType)
{    
    initKey(SortKeyMaskInterfaces[maskType], (SortKeyData)maskType);

    // Checks for array above.
    SF_COMPILER_ASSERT(SortKeyMask_Push == 0);
    SF_COMPILER_ASSERT(SortKeyMask_Pop == 2);
}

// This function simply provides a method to check within a range if a mask is present. This is used
// by CacheableSKIs, to determine whether a depth/stencil target is required for the masking.
bool isMaskPresentInsideRange(const BundleIterator& ibundles, SortKeyType startKey, SortKeyType endKey)
{
    // Iterator through the remainder of the bundles, and see if masks are present.
    bool maskPresent = false;
    BundleIterator ib = ibundles;
    int cachedPrimLevel = 0;
    while(ib)
    {
        if (cachedPrimLevel == 1 &&
            (ib->Key.GetType() == SortKey_MaskStart ||
            ib->Key.GetType() == SortKey_MaskEnd ||
            ib->Key.GetType() == SortKey_Text))    // NOTE: textfields may have text clipping, which acts as a 'real' mask.
        {
            maskPresent = true;
            break;
        }
        else if (ib->Key.GetType() == endKey)
        {
            cachedPrimLevel--;
            if ( cachedPrimLevel <= 0 )
                break;
        }
        else if (ib->Key.GetType() == startKey)
        {
            cachedPrimLevel++;
        }
        ib++;
    }
    return maskPresent;
}

//--------------------------------------------------------------------
// ***** SortKey Interfaces for BlendMode

class SKI_BlendMode : public SortKeyInterface
{
public:
    SKI_BlendMode(SortKeyType type, unsigned flags) : SortKeyInterface(type, flags)
    { }

    static SKI_BlendMode  Start_Instance;
    static SKI_BlendMode  StartCacheable_Instance;
    static SKI_BlendMode  End_Instance;

    virtual SortKeyTransition GetRangeTransition(SortKeyData, const SortKey& other)
    {
        if ((other.GetType() == SortKey_BlendModeEnd) && (Type == SortKey_BlendModeStart))
            return SKT_End;
        return SKT_Skip;
    }

    virtual bool UpdateBundleEntry(SortKeyData d, BundleEntry* p,
        TreeCacheRoot* tr, Renderer2DImpl*,
        const BundleIterator& ibundles)
    {
        if (!p->pBundle)
        {
            BlendMode mode = (BlendMode)reinterpret_cast<UPInt>(d);
            // Only need to check if the mask is present if the blend mode requires a target.
            Ptr<BlendModeBundle> bundle = *SF_HEAP_AUTO_NEW(tr) BlendModeBundle(mode, 
                BlendState::IsTargetAllocationNeededForBlendMode(mode) ? 
                isMaskPresentInsideRange(ibundles, SortKey_BlendModeStart, SortKey_BlendModeEnd) : false);
            p->SetBundle(bundle, 0);
        }
        return p->pBundle.GetPtr() != 0;
    }

    virtual void DrawBundleEntry(SortKeyData, BundleEntry*p, Renderer2DImpl* r2d)
    {
        SF_AMP_SCOPE_RENDER_TIMER("SKI_BlendMode::DrawBundleEntry", Amp_Profile_Level_High);
        if (p->pBundle)
        {
            BlendModeBundle*bundle = (BlendModeBundle*)p->pBundle.GetPtr();
            bundle->Draw(r2d->GetHAL());
        }
    }
};


// Keys
SKI_BlendMode SKI_BlendMode::Start_Instance(SortKey_BlendModeStart, SKF_RangeStart|SKF_MatchNoOverlap);
SKI_BlendMode SKI_BlendMode::StartCacheable_Instance(SortKey_BlendModeStart, SKF_RangeStart|SKF_MatchNever);
SKI_BlendMode SKI_BlendMode::End_Instance(SortKey_BlendModeEnd, SKF_MatchNoOverlap);

// Blend Mode SortKey constructor (non-cacheable blends).
SortKey::SortKey(SortKeyType blendKeyType, BlendMode mode)
{    
    SF_DEBUG_ASSERT1((blendKeyType == SortKey_BlendModeStart) || (blendKeyType == SortKey_BlendModeEnd),
        "Unexpected blendKeyType (%d) - should be either SortKey_BlendModeStart or SortKey_BlendModeEnd", blendKeyType);
    
    if (blendKeyType == SortKey_BlendModeStart)
    {
        // Use a SKI that will never match, if it is a blend mode that requires target. These currently cannot batch.
        if (BlendState::IsTargetAllocationNeededForBlendMode(mode))
            initKey(&SKI_BlendMode::StartCacheable_Instance, (SortKeyData)mode);
        else
            initKey(&SKI_BlendMode::Start_Instance, (SortKeyData)mode);
    }
    else
    {
        initKey(&SKI_BlendMode::End_Instance, (SortKeyData)mode);
    }
}

//--------------------------------------------------------------------
// ***** SortKey Interfaces for Filter

class SKI_Filter : public SortKeyInterface_RefCountImpl
{
public:
    SKI_Filter(SortKeyType type, unsigned flags) : SortKeyInterface_RefCountImpl(type, flags)
    { }

    static SKI_Filter  Start_Instance;
    static SKI_Filter  End_Instance;

    virtual SortKeyTransition GetRangeTransition(SortKeyData, const SortKey& other)
    {
        if ((other.GetType() == SortKey_FilterEnd) && (Type == SortKey_FilterStart))
            return SKT_End;
        return SKT_Skip;
    }

    virtual bool UpdateBundleEntry(SortKeyData d, BundleEntry* p,
                                   TreeCacheRoot* tr, Renderer2DImpl*,
                                   const BundleIterator& ibundles)
    {
        if (!p->pBundle)
        {
            Ptr<FilterBundle> bundle = *SF_HEAP_AUTO_NEW(tr) FilterBundle((FilterSet*)d, 
                isMaskPresentInsideRange(ibundles, SortKey_FilterStart, SortKey_FilterEnd));
            p->SetBundle(bundle, 0);
        }
        return p->pBundle.GetPtr() != 0;
    }

    virtual void DrawBundleEntry(SortKeyData, BundleEntry* p, Renderer2DImpl* r2d)
    {
        SF_AMP_SCOPE_RENDER_TIMER("SKI_Filter::DrawBundleEntry", Amp_Profile_Level_High);
        if (p->pBundle)
        {
            FilterBundle *bundle = (FilterBundle *)p->pBundle.GetPtr();
            bundle->Draw(r2d->GetHAL());
        }
    }
};


// Keys
SKI_Filter SKI_Filter::Start_Instance(SortKey_FilterStart, SKF_RangeStart|SKF_MatchNoOverlap);
SKI_Filter SKI_Filter::End_Instance  (SortKey_FilterEnd,   SKF_MatchNoOverlap);

// Filter Mode SortKey constructor.
SortKey::SortKey(SortKeyType filterKeyType, const FilterSet* filters)
{    
    SF_ASSERT((filterKeyType == SortKey_FilterStart) || (filterKeyType == SortKey_FilterEnd));
    
    if (filterKeyType == SortKey_FilterStart)
        initKey(&SKI_Filter::Start_Instance, (SortKeyData)filters);
    else
        initKey(&SKI_Filter::End_Instance, (SortKeyData)0);
}

//--------------------------------------------------------------------
// ***** SortKey Interfaces for ViewMatrix3D

class SKI_ViewMatrix3D : public SortKeyInterface
{
public:
    SKI_ViewMatrix3D(SortKeyType type, unsigned flags) : SortKeyInterface(type, flags)
    { }

    static SKI_ViewMatrix3D  Start_Instance;
    static SKI_ViewMatrix3D  End_Instance;

    virtual SortKeyTransition GetRangeTransition(SortKeyData, const SortKey& other)
    {
        if ((other.GetType() == SortKey_ViewMatrix3DEnd) && (Type == SortKey_ViewMatrix3DStart))
            return SKT_End;
        return SKT_Skip;
    }

    virtual bool UpdateBundleEntry(SortKeyData d, BundleEntry* p, TreeCacheRoot* tr, 
                                   Renderer2DImpl*, const BundleIterator&)
    {
        if (!p->pBundle)
        {
            Ptr<ViewMatrix3DBundle> bundle = *SF_HEAP_AUTO_NEW(tr) ViewMatrix3DBundle((Matrix3FRef*)d);
            p->SetBundle(bundle, 0);
        }
        return p->pBundle.GetPtr() != 0;
    }

    virtual void DrawBundleEntry(SortKeyData, BundleEntry* p, Renderer2DImpl* r2d)
    {
        SF_AMP_SCOPE_RENDER_TIMER("SKI_ViewMatrix3D::DrawBundleEntry", Amp_Profile_Level_High);
        if (p->pBundle)
        {
            ViewMatrix3DBundle *bundle = (ViewMatrix3DBundle *)p->pBundle.GetPtr();
            bundle->Draw(r2d->GetHAL());
        }
    }
};


// Keys
SKI_ViewMatrix3D SKI_ViewMatrix3D::Start_Instance(SortKey_ViewMatrix3DStart, SKF_RangeStart|SKF_MatchNoOverlap);
SKI_ViewMatrix3D SKI_ViewMatrix3D::End_Instance  (SortKey_ViewMatrix3DEnd,   SKF_MatchNoOverlap);

// ViewMatrix3D SortKey constructor.
SortKey::SortKey(SortKeyType vm3DKeyType, const Matrix3FRef* viewMat)
{    
    SF_ASSERT((vm3DKeyType == SortKey_ViewMatrix3DStart) || (vm3DKeyType == SortKey_ViewMatrix3DEnd));

    if (vm3DKeyType == SortKey_ViewMatrix3DStart)
        initKey(&SKI_ViewMatrix3D::Start_Instance, (SortKeyData)viewMat);
    else
        initKey(&SKI_ViewMatrix3D::End_Instance, (SortKeyData)0);
}

//--------------------------------------------------------------------
// ***** SortKey Interfaces for ProjectionMatrix3D

class SKI_ProjectionMatrix3D : public SortKeyInterface
{
public:
    SKI_ProjectionMatrix3D(SortKeyType type, unsigned flags) : SortKeyInterface(type, flags)
    { }

    static SKI_ProjectionMatrix3D  Start_Instance;
    static SKI_ProjectionMatrix3D  End_Instance;

    virtual SortKeyTransition GetRangeTransition(SortKeyData, const SortKey& other)
    {
        if ((other.GetType() == SortKey_ProjectionMatrix3DEnd) && (Type == SortKey_ProjectionMatrix3DStart))
            return SKT_End;
        return SKT_Skip;
    }

    virtual bool UpdateBundleEntry(SortKeyData d, BundleEntry* p, TreeCacheRoot* tr, 
                                   Renderer2DImpl*, const BundleIterator&)
    {
        if (!p->pBundle)
        {
            Ptr<ProjectionMatrix3DBundle> bundle = *SF_HEAP_AUTO_NEW(tr) ProjectionMatrix3DBundle((Matrix4FRef*)d);
            p->SetBundle(bundle, 0);
        }
        return p->pBundle.GetPtr() != 0;
    }

    virtual void DrawBundleEntry(SortKeyData, BundleEntry* p, Renderer2DImpl* r2d)
    {
        SF_AMP_SCOPE_RENDER_TIMER("SKI_ProjectionMatrix3D::DrawBundleEntry", Amp_Profile_Level_High);
        if (p->pBundle)
        {
            ProjectionMatrix3DBundle *bundle = (ProjectionMatrix3DBundle *)p->pBundle.GetPtr();
            bundle->Draw(r2d->GetHAL());
        }
    }
};


// Keys
SKI_ProjectionMatrix3D SKI_ProjectionMatrix3D::Start_Instance(SortKey_ProjectionMatrix3DStart, SKF_RangeStart|SKF_MatchNoOverlap);
SKI_ProjectionMatrix3D SKI_ProjectionMatrix3D::End_Instance  (SortKey_ProjectionMatrix3DEnd,   SKF_MatchNoOverlap);

// ProjectionMatrix3D SortKey constructor.
SortKey::SortKey(SortKeyType pm3DKeyType, const Matrix4FRef* projMat)
{    
    SF_ASSERT((pm3DKeyType == SortKey_ProjectionMatrix3DStart) || (pm3DKeyType == SortKey_ProjectionMatrix3DEnd));

    if (pm3DKeyType == SortKey_ProjectionMatrix3DStart)
        initKey(&SKI_ProjectionMatrix3D::Start_Instance, (SortKeyData)projMat);
    else
        initKey(&SKI_ProjectionMatrix3D::End_Instance, (SortKeyData)0);
}


//--------------------------------------------------------------------
// ***** SortKey Interfaces for UserData

class SKI_UserData : public SortKeyInterface_RefCountImpl
{
public:
    SKI_UserData(SortKeyType type, unsigned flags) : SortKeyInterface_RefCountImpl(type, flags)
    { }

    static SKI_UserData Start_Instance;
    static SKI_UserData End_Instance;

    virtual SortKeyTransition GetRangeTransition(SortKeyData, const SortKey& other)
    {
        if ((other.GetType() == SortKey_UserDataEnd) && (Type == SortKey_UserDataStart))
            return SKT_End;
        return SKT_Skip;
    }

    virtual bool UpdateBundleEntry(SortKeyData d, BundleEntry* p, TreeCacheRoot* tr, 
        Renderer2DImpl*, const BundleIterator&)
    {
        if (!p->pBundle)
        {
            Ptr<UserDataBundle> bundle = *SF_HEAP_AUTO_NEW(tr) UserDataBundle((UserDataState::Data*)d);
            p->SetBundle(bundle, 0);
        }
        return p->pBundle.GetPtr() != 0;
    }

    virtual void DrawBundleEntry(SortKeyData, BundleEntry* p, Renderer2DImpl* r2d)
    {
        SF_AMP_SCOPE_RENDER_TIMER(__FUNCTION__, Amp_Profile_Level_High);
        if (p->pBundle)
        {
            UserDataBundle *bundle = (UserDataBundle *)p->pBundle.GetPtr();
            bundle->Draw(r2d->GetHAL());
        }
    }

    // Overridden Equal function, so that we can disable batching.
    virtual bool AcceptMatch(SortKeyData d0, SortKeyData d1)
    {
        if (d0 != d1)
            return false;

        // If pointers are equal, we only need to actually inspect one object.
        UserDataState::Data* ud0 = reinterpret_cast<UserDataState::Data*>(d0);
        if (ud0 == 0)
            return true;
        return ((ud0->Flags & UserDataState::Data::Data_Batching) == 0) || !ud0->BatchingDisabled;
    }
};


// Keys
SKI_UserData SKI_UserData::Start_Instance(SortKey_UserDataStart, SKF_RangeStart|SKF_MatchNoOverlap);
SKI_UserData SKI_UserData::End_Instance  (SortKey_UserDataEnd,   SKF_MatchNoOverlap);


// UserData SortKey constructor.
SortKey::SortKey(SortKeyType keyType, UserDataState::Data* data)
{    
    SF_ASSERT((keyType == SortKey_UserDataStart) || (keyType == SortKey_UserDataEnd));
    if (keyType == SortKey_UserDataStart)
        initKey(&SKI_UserData::Start_Instance, (SortKeyData)data);
    else
        initKey(&SKI_UserData::End_Instance, (SortKeyData)0);
}

}} // Scaleform::Render
