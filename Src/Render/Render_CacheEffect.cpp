/**************************************************************************

Filename    :   Render_CacheEffect.cpp
Content     :   Implements CacheEffect and CacheEffectChain classes
                attachable to TreeCacheNode for mask, blend mode and
                other effect support.
Created     :   July 8, 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/Render_CacheEffect.h"
#include "Render/Render_TreeCacheNode.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {

//--------------------------------------------------------------------
// ***** CacheEffect

void CacheEffect::ChainNextDefault(BundleEntryRange* chain, BundleEntry& StartEntry, BundleEntry& EndEntry)
{
    StartEntry.SetNextPattern(chain->GetFirst());
    StartEntry.pChain = 0;
    StartEntry.ChainHeight = 0;

    chain->GetLast()->SetNextPattern(&EndEntry);
    EndEntry.pChain = 0;
    EndEntry.ChainHeight = 0;

    Length = chain->GetLength() + 2;
    chain->Init(&StartEntry, &EndEntry, Length);
}

//--------------------------------------------------------------------
// ***** CacheEffectChain


struct ChainOrderRecord
{
    StateType               Type;
    unsigned                ChangeFlag;
    CacheEffect::CreateFunc Factory;
};

// This sequence determines the order in which these effects are applied (first in the list is applied to the scene first). 
// The ordering of some components matters for correct visualization with flash content. The order of the first three 
// (Blend, Filter, Mask) are particularly important.
ChainOrderRecord ChainOrderSequence[] = 
{
    { State_BlendMode,          Change_State_BlendMode,         &BlendModeEffect::Create },
    { State_Filter,             Change_State_Filter,            &FilterEffect::Create },
    { State_MaskNode,           Change_State_MaskNode,          &MaskEffect::Create },
    { State_ViewMatrix3D,       Change_State_ViewMatrix3D,      &ViewMatrix3DEffect::Create },
    { State_ProjectionMatrix3D, Change_State_ProjectionMatrix3D,&ProjectionMatrix3DEffect::Create },
    { State_UserData,           Change_State_UserData,          &UserDataEffect::Create }
};

enum { ChainOrderSequenzeSize = sizeof(ChainOrderSequence) / sizeof (ChainOrderRecord) };


CacheEffectChain::~CacheEffectChain()
{
    // Delete effects.
    CacheEffect *effect = pEffect, *temp;
    while (effect)
    {
        temp = effect;
        effect = effect->pNext;
        delete temp;
    }
}

bool CacheEffectChain::UpdateEffects(TreeCacheNode* node, unsigned changeFlags)
{
    bool updateParent = false;

    // Walk through sequence in order verifying presence/creating effects.
    if (!node->pNode)
        return false;
    const TreeNode::NodeData* treeNodeData = node->pNode->GetDisplayData();
    const StateBag&           states = treeNodeData->States;

    CacheEffect**prevEffectPointer = &pEffect;
    CacheEffect *effect = pEffect;
    UPInt        iseq;
    
    // Short-circuit: Don't do iteration if states are empty.
    if (!effect && states.IsEmpty())
        return false;

    for (iseq = 0; iseq < ChainOrderSequenzeSize; iseq++)
    {
        ChainOrderRecord& r = ChainOrderSequence[iseq];

        if (effect && (effect->GetType() == r.Type))
        {
            CacheEffect* nextEffect = effect->pNext;
            
            if (changeFlags & r.ChangeFlag)
            {
                const State* state = states.GetState(r.Type);
                if (!state)
                {
                    // Remove effect
                    // Kill List seems unnecessary since parent will not
                    // access us before chains are stripped.
                    delete effect;
                    *prevEffectPointer = nextEffect;
                    updateParent = true;
                    effect = nextEffect;
                    continue;
                }
                
                updateParent |= effect->Update(state);
            }
            prevEffectPointer = &effect->pNext;
            effect = nextEffect;
            continue;
        }
        else
        {
            // Effect of this index not present, which means we may need to create
            // a new effect.
            if (changeFlags & r.ChangeFlag)
            {
                const State* state = states.GetState(r.Type);
                if (state)
                {
                    CacheEffect* newEffect = r.Factory(node, state, effect);
                    if (newEffect)
                    {
                        *prevEffectPointer = newEffect;
                        prevEffectPointer = &newEffect->pNext;
                    }
                }
            }
        }
    }

    return updateParent;
}

void CacheEffectChain::updateBundleChain(CacheEffect* effect,
                                         BundleEntryRange* chain,
                                         BundleEntryRange* maskChain)
{    
    if (effect->pNext)
        updateBundleChain(effect->pNext, chain, maskChain);
    effect->ChainNext(chain, maskChain);
}



//--------------------------------------------------------------------
// ***** Mask Effect

MaskEffect::MaskEffect(TreeCacheNode* node,
                       const MaskEffectState mes, const HMatrix& areaMatrix,
                       CacheEffect* next)
: CacheEffect(next),
  StartEntry(node, SortKey(SortKeyMask_Push)),
  EndEntry(node, SortKey(SortKeyMask_End)),
  PopEntry(node, SortKey(SortKeyMask_Pop)),
  MES(mes), BoundsMatrix(areaMatrix)
{
}

bool MaskEffect::UpdateMatrix(const MaskEffectState mes,
                              const Matrix2F& areaMatrix)
{
    BoundsMatrix.SetMatrix2D(areaMatrix);
    // If masking state changed, mark us for update, which can
    // actually change the state key.
    return (mes != MES);
}

bool MaskEffect::Update(const State* stateArg)
{    
    SF_DEBUG_ASSERT(stateArg->GetType() == State_MaskNode, "Updating MaskEffect, stateArg must be a MaskNode state.");
    SF_UNUSED(stateArg);
    
    bool updateParent = false;

    // Update re-calculates the node mostly to see if key-type has
    // changed, in which case it is re-updated. This update is slightly
    // redundant with UpdateMatrix, which marks us for update
    // when MaskEffectState changes (TBD: Could it do this directly?). 

    TreeCacheNode* node = StartEntry.pSourceNode;
    SF_DEBUG_ASSERT(node, "Updating MaskEffect, previous key did not have a source node.");
    if (!node)
        return false;
    
    RectF       maskBounds;
    Matrix2F    maskAreaMatrix;
    Matrix3F    viewMatrix;
    Matrix4F    viewProjMatrix;
    node->CalcViewMatrix(&viewMatrix, &viewProjMatrix);
    MaskEffectState mes = node->calcMaskBounds(&maskBounds, &maskAreaMatrix,
                                               viewMatrix, viewProjMatrix);

    SortKeyType     keyType          = SortKey_MaskStart;
    SortKeyMaskType keyConstructType = SortKeyMask_Push;

    // If type changes, we need to re-flush the bundle.
    if (keyType != StartEntry.Key.GetType())
    {
        StartEntry.ClearBundle();
        StartEntry.Key = SortKey(keyConstructType);
        // Logically, we should re-update parent since our sorting changed.
        SF_ASSERT(node->pRoot);
        // However, Update/UpdateEffects is only called from GetPatternChain, which implies
        // that parent is being rebuilt anyway. This means that we don't need to add it to
        // update list; furthermore, if we did it we could corrupt nodes UpdateFlags after
        // the depth has already been processed (resulting in carryover and corruption
        // in future frames).
        // Do NOT do this: pnode->addParentToDepthPatternUpdate()
        updateParent = true; // Redundant? Need investigation...
    }

    MES = mes;
    BoundsMatrix.SetMatrix2D(maskAreaMatrix);
    return updateParent;
}

void MaskEffect::ChainNext(BundleEntryRange* chain, BundleEntryRange* maskChain)
{
    if ((MES == MES_NoMask) || !maskChain || (maskChain->GetLength() == 0))
    {
        // Mark empty chain with StartEntry.pNextPattern = 0;
        // record chain values in EndEntry/Pop.
        StartEntry.SetNextPattern(0);
        EndEntry.SetNextPattern(chain->GetFirst());
        PopEntry.SetNextPattern(chain->GetLast());
        Length = chain->GetLength();
        // Chain remains unchanged.
    }
    else
    {
        StartEntry.SetNextPattern(maskChain->GetFirst());
        StartEntry.pChain = 0;
        StartEntry.ChainHeight = 0;
        
        maskChain->GetLast()->SetNextPattern(&EndEntry);

        EndEntry.SetNextPattern(chain->GetFirst());
        EndEntry.pChain = 0;
        EndEntry.ChainHeight = 0;

        chain->GetLast()->SetNextPattern(&PopEntry);
        PopEntry.pChain = 0;
        PopEntry.ChainHeight = 0;

        Length = chain->GetLength() + maskChain->GetLength() + 3;
        chain->Init(&StartEntry, &PopEntry, Length);
    }
}

void  MaskEffect::GetRange(BundleEntryRange* result)
{
    if (StartEntry.HasNextPattern())
        result->Init(&StartEntry, &PopEntry, Length);
    else
        result->Init(EndEntry.GetNextPattern(), PopEntry.GetNextPattern(), Length); 
}


CacheEffect* MaskEffect::Create(TreeCacheNode* node, const State*, CacheEffect* next)
{
    // Determine what type of mask to create (clipped or not).
    RectF       maskBounds;
    Matrix2F    maskAreaMatrix;
    Matrix3F    viewMatrix;
    Matrix4F    viewProjMatrix;
    node->CalcViewMatrix(&viewMatrix, &viewProjMatrix);
    MaskEffectState mes = node->calcMaskBounds(&maskBounds, &maskAreaMatrix,
                                               viewMatrix, viewProjMatrix);
    HMatrix     m = node->GetMatrixPool().CreateMatrix(maskAreaMatrix);

    return SF_HEAP_AUTO_NEW(node) MaskEffect(node, mes, m, next);
}



//--------------------------------------------------------------------
// ***** BlendMode Effect

BlendModeEffect::BlendModeEffect(TreeCacheNode* node, const HMatrix& m, const BlendState& state, CacheEffect* next)
:   CacheEffect(next),
    StartEntry(node, SortKey(SortKey_BlendModeStart, state.GetBlendMode())),
    EndEntry(node, SortKey(SortKey_BlendModeEnd, BlendState::IsTargetAllocationNeededForBlendMode(state.GetBlendMode()) ? Blend_Invalid : Blend_Count)),
    BoundsMatrix(m)
{
}

bool BlendModeEffect::UpdateMatrix(const Matrix2F& boundsMatrix)
{
    // Update the matrix. TBD: we may want to support changing the bounds of the blend 
    // operation. If this is the case, this function will need to be more complex.
    BoundsMatrix.SetMatrix2D(boundsMatrix);     
    return true;
}


void BlendModeEffect::UpdateCxform(const Cxform& cx)
{
    BoundsMatrix.SetCxform(cx);
}

bool BlendModeEffect::Update(const State* stateArg)
{
    SF_ASSERT(stateArg->GetType() == State_BlendMode);
    BlendState* state = (BlendState*)stateArg;

    // Detect when the blend is/was not cacheable, and simply change the mode.
    bool targetNeeded = BlendState::IsTargetAllocationNeededForBlendMode(state->GetBlendMode());

    // Rebuild the bundle. The blend may not have changed, but child content may
    // have transformed, in which case the blend will need to be re-rendered. 
    StartEntry.ClearBundle();
    EndEntry.ClearBundle();

    // Note: EndEntry use different blend values. This is to differentiate during the unprepare phase.
    StartEntry.Key = SortKey(SortKey_BlendModeStart, state->GetBlendMode());
    EndEntry.Key = SortKey(SortKey_BlendModeEnd, targetNeeded ? Blend_Invalid : Blend_Count);
    return true;
}

void  BlendModeEffect::ChainNext(BundleEntryRange* chain, BundleEntryRange*)
{
    CacheEffect::ChainNextDefault(chain, StartEntry, EndEntry);
}

void  BlendModeEffect::GetRange(BundleEntryRange* result)
{
    result->Init(&StartEntry, &EndEntry, Length);
}

CacheEffect* BlendModeEffect::Create(TreeCacheNode* node, const State* stateArg, CacheEffect* next)
{
    SF_ASSERT(stateArg->GetType() == State_BlendMode);

    Matrix3F    viewMatrix;
    Matrix4F    viewProjMatrix;
    Matrix2F    areaMatrix;
    RectF       blendBounds;
    node->CalcViewMatrix(&viewMatrix, &viewProjMatrix);
    if (!node->calcCacheableBounds(&blendBounds, &areaMatrix, viewMatrix, viewProjMatrix ))
    {
        // If we fail to calculate the filter bounds, reset the matrix and bounds, as they may be invalid.
        blendBounds.SetRect(0,0);
        areaMatrix.SetIdentity();
    }

    HMatrix     m = node->GetMatrixPool().CreateMatrix(areaMatrix);
    const TreeNode::NodeData* nodeData = node->GetNodeData();
    m.SetCxform(nodeData->Cx);

    return SF_HEAP_AUTO_NEW(node) BlendModeEffect(node, m, *(BlendState*)stateArg, next);
}

//--------------------------------------------------------------------
// ***** FilterMode Effect

FilterEffect::FilterEffect(TreeCacheNode* node, const HMatrix& m, const FilterState& state, CacheEffect* next)
: CacheEffect(next),
  Contributing(true), 
  StartEntry(node, SortKey(SortKey_FilterStart, state.GetFilters())),
  EndEntry(node, SortKey(SortKey_FilterEnd, (FilterSet*)0)),
  BoundsMatrix(m)
{
    Matrix2F zero;
    zero.SetZero();
    BoundsMatrix.SetTextureMatrix(zero, 1);

    // Store the original as a UserData, don't want to modify HMatrix just for this one case.
    BoundsMatrix.SetUserDataFloat(&(m.GetMatrix2D().M[0][0]), 8);

    // Create a GradientImage from the GradientData.
    for (unsigned i = 0; i < state.GetFilterCount(); ++i)
    {
        const Filter* filter = state.GetFilter(i);
        if (filter->GetFilterType() == Filter_GradientBevel ||
            filter->GetFilterType() == Filter_GradientGlow)
        {
            const GradientFilter* gradientFilter = reinterpret_cast<const GradientFilter*>(filter);
            gradientFilter->GenerateGradientImage(node->GetPrimitiveFillManager());
        }
    }
}

bool FilterEffect::UpdateMatrix(const Matrix2F& boundsMatrix, const Matrix2F& newNodeMatrix, bool forceUncache) 
{ 
    bool willCache = false;
    const Matrix2F& nodeMatrix = BoundsMatrix.GetTextureMatrix(1);
    Matrix2F zero;
    zero.SetZero();
    if (!forceUncache && BoundsMatrix.HasTextureMatrix(1) && nodeMatrix != zero)
    {
        // We inspect the previous node matrix, and the new one. If there is a compatible transformation,
        // we can cache filters across the transformation. / Decompose both matrices, and see if there 
        // has been a change that requires updating the (potentially cached) filter.
        const float tolerance = 0.0001f;
        
        
        bool deltaTrans = !Alg::ToleranceEqual(newNodeMatrix.Tx(), nodeMatrix.Tx(), tolerance) || 
                          !Alg::ToleranceEqual(newNodeMatrix.Ty(), nodeMatrix.Ty(), tolerance);
        bool deltaScale = !Alg::ToleranceEqual(newNodeMatrix.GetXScale(), nodeMatrix.GetXScale(), tolerance) ||
                          !Alg::ToleranceEqual(newNodeMatrix.GetYScale(), nodeMatrix.GetYScale(), tolerance);
        bool deltaRot   = !Alg::ToleranceEqual(newNodeMatrix.GetRotation(), nodeMatrix.GetRotation(), tolerance);

        // Also compare the bounds matrices, to see if they have changed. Likely the only one that would be different
        // between the node matrices is scale, as the filter/CaB primitive could change shape, but remain in the same place.
        const Matrix2F& oldBoundsMtx = BoundsMatrix.GetMatrix2D();
        deltaTrans |=   !Alg::ToleranceEqual(boundsMatrix.Tx(), oldBoundsMtx.Tx(), tolerance) || 
                        !Alg::ToleranceEqual(boundsMatrix.Ty(), oldBoundsMtx.Ty(), tolerance);
        deltaScale |=   !Alg::ToleranceEqual(boundsMatrix.GetXScale(), oldBoundsMtx.GetXScale(), tolerance) ||
                        !Alg::ToleranceEqual(boundsMatrix.GetYScale(), oldBoundsMtx.GetYScale(), tolerance);
        deltaRot   |=   !Alg::ToleranceEqual(boundsMatrix.GetRotation(), oldBoundsMtx.GetRotation(), tolerance);

        const FilterSet* filters = reinterpret_cast<const FilterSet*>(StartEntry.Key.GetData());
        willCache = filters->CanCacheAcrossTransform(deltaTrans, deltaRot, deltaScale);
    }

    if (willCache)
    {
        Matrix2F finalBounds(BoundsMatrix.GetUserDataFloat());
        Matrix2F deltaMatrix = BoundsMatrix.GetTextureMatrix(1);
        deltaMatrix.Invert();
        deltaMatrix.Append(newNodeMatrix);
        finalBounds.Append(deltaMatrix);

        // Update the matrix.
        BoundsMatrix.SetMatrix2D(finalBounds); 

        // Save the unmodified bounds matrix in an unused part of the HMatrix. This matrix may be 
        // required if the filter becomes uncached.
        BoundsMatrix.SetTextureMatrix(boundsMatrix, 0);
        
    }
    else
    {
        // Save the new node matrix, as we are not going to cache, so any subsequent transformations
        // should based their new cached position from that coordinate space.
        BoundsMatrix.SetTextureMatrix(newNodeMatrix, 1);

        // Store the original as a UserData, don't want to modify HMatrix just for this one case.
        BoundsMatrix.SetUserDataFloat(&(boundsMatrix.M[0][0]), 8);

        // Update the matrix.
        BoundsMatrix.SetMatrix2D(boundsMatrix); 
        BoundsMatrix.SetTextureMatrix(boundsMatrix, 0); 
    }

    return !willCache;
}

void FilterEffect::UpdateCxform(const Cxform& cx)
{
    BoundsMatrix.SetCxform(cx);
}

bool FilterEffect::Update(const State* stateArg)
{
    SF_ASSERT(stateArg->GetType() == State_Filter);
    FilterState* state = (FilterState*)stateArg;

    // Always rebuild the bundle. The filters may not have changed, but child content may
    // have transformed, in which case the filter will need to be recalculated. 
    StartEntry.ClearBundle();
    EndEntry.ClearBundle();

    StartEntry.Key = SortKey(SortKey_FilterStart, state->GetFilters());
    EndEntry.Key = SortKey(SortKey_FilterEnd, (const FilterSet*)0);
    return true;
}

void FilterEffect::ChainNext(BundleEntryRange* chain, BundleEntryRange*)
{
    // If the filter does not contribute visually, then don't insert it into the chain
    const FilterSet* filters = (const FilterSet*)this->StartEntry.Key.GetData();

    Contributing = filters && filters->IsContributing();

    if ( Contributing )
    {
        CacheEffect::ChainNextDefault(chain, StartEntry, EndEntry);
    }
    else
    {
        // Chain is unmodified (filter had no effect)
        StartEntry.SetNextPattern(chain->GetFirst());
        EndEntry.SetNextPattern(chain->GetLast());
        Length = chain->GetLength();
    }
}

void  FilterEffect::GetRange(BundleEntryRange* result)
{
    if ( Contributing )
        result->Init(&StartEntry, &EndEntry, Length);
    else
        result->Init(StartEntry.GetNextPattern(), EndEntry.GetNextPattern(), Length);
}

CacheEffect* FilterEffect::Create(TreeCacheNode* node, const State* stateArg, CacheEffect* next)
{
    RectF       filterBounds;
    Matrix2F    filterAreaMatrix;
    Cxform      filterCx;
    Matrix3F    viewMatrix;
    Matrix4F    viewProjMatrix;
    node->CalcViewMatrix(&viewMatrix, &viewProjMatrix);
    node->CalcCxform(filterCx);
    if (!node->calcCacheableBounds(&filterBounds, &filterAreaMatrix, viewMatrix, viewProjMatrix ))
	{
        // If we fail to calculate the filter bounds, reset the matrix and bounds, as they may be invalid.
		filterBounds.SetRect(0,0);
        filterAreaMatrix.SetIdentity();
	}

    HMatrix     m = node->GetMatrixPool().CreateMatrix(filterAreaMatrix);
    m.SetCxform(filterCx);

    SF_ASSERT(stateArg->GetType() == State_Filter);
    return SF_HEAP_AUTO_NEW(node) FilterEffect(node, m, *(FilterState*)stateArg, next);
}

//--------------------------------------------------------------------
// ***** ViewMatrix3DEffect

ViewMatrix3DEffect::ViewMatrix3DEffect(TreeCacheNode* node, const ViewMatrix3DState& state, CacheEffect* next)
: CacheEffect(next),
StartEntry(node, SortKey(SortKey_ViewMatrix3DStart, state.GetViewMatrix3D())),
EndEntry(node, SortKey(SortKey_ViewMatrix3DEnd, (Matrix3FRef*)NULL))
{
}

bool ViewMatrix3DEffect::Update(const State* stateArg)
{
    SF_ASSERT(stateArg->GetType() == State_ViewMatrix3D);
    ViewMatrix3DState* state = (ViewMatrix3DState*)stateArg;

    // Always rebuild the bundle. 
    StartEntry.ClearBundle();
    StartEntry.Key = SortKey(SortKey_ViewMatrix3DStart, state->GetViewMatrix3D());
    return true;
}

void  ViewMatrix3DEffect::ChainNext(BundleEntryRange* chain, BundleEntryRange*)
{
    CacheEffect::ChainNextDefault(chain, StartEntry, EndEntry);
}

void  ViewMatrix3DEffect::GetRange(BundleEntryRange* result)
{
    result->Init(&StartEntry, &EndEntry, Length);
}

CacheEffect* ViewMatrix3DEffect::Create(TreeCacheNode* node, const State* stateArg, CacheEffect* next)
{
    SF_ASSERT(stateArg->GetType() == State_ViewMatrix3D);
    ViewMatrix3DState* state = (ViewMatrix3DState*)stateArg;
    return SF_HEAP_AUTO_NEW(node) ViewMatrix3DEffect(node, *state, next);
}

//--------------------------------------------------------------------
// ***** ProjectionMatrix3DEffect

ProjectionMatrix3DEffect::ProjectionMatrix3DEffect(TreeCacheNode* node, const ProjectionMatrix3DState& state, CacheEffect* next)
: CacheEffect(next),
StartEntry(node, SortKey(SortKey_ProjectionMatrix3DStart, state.GetProjectionMatrix3D())), 
EndEntry(node, SortKey(SortKey_ProjectionMatrix3DEnd, (Matrix4FRef*)NULL))
{
//    ProjMatrix = state.GetProjectionMatrix3D()->M;    // copy projection Matrix
}

bool ProjectionMatrix3DEffect::Update(const State* stateArg)
{
    SF_ASSERT(stateArg->GetType() == State_ProjectionMatrix3D);
    ProjectionMatrix3DState* state = (ProjectionMatrix3DState*)stateArg;

    // Always rebuild the bundle.
    StartEntry.ClearBundle();
    StartEntry.Key = SortKey(SortKey_ProjectionMatrix3DStart, state->GetProjectionMatrix3D());
    return true;
}

void  ProjectionMatrix3DEffect::ChainNext(BundleEntryRange* chain, BundleEntryRange*)
{
    CacheEffect::ChainNextDefault(chain, StartEntry, EndEntry);
}

void  ProjectionMatrix3DEffect::GetRange(BundleEntryRange* result)
{
    result->Init(&StartEntry, &EndEntry, Length);
}

CacheEffect* ProjectionMatrix3DEffect::Create(TreeCacheNode* node, const State* stateArg, CacheEffect* next)
{
    SF_ASSERT(stateArg->GetType() == State_ProjectionMatrix3D);
    ProjectionMatrix3DState* state = (ProjectionMatrix3DState*)stateArg;
    return SF_HEAP_AUTO_NEW(node) ProjectionMatrix3DEffect(node, *state, next);
}

//--------------------------------------------------------------------
// ***** UserDataEffect

UserDataEffect::UserDataEffect(TreeCacheNode* node, const UserDataState& state, CacheEffect* next)
: CacheEffect(next),
    StartEntry(node, SortKey(SortKey_UserDataStart, (UserDataState::Data*)0)), 
    EndEntry  (node, SortKey(SortKey_UserDataEnd,   (UserDataState::Data*)0)) 
{
    rebuildBundles(state);
}

bool UserDataEffect::Update(const State* stateArg)
{
    SF_ASSERT(stateArg->GetType() == State_UserData);
    UserDataState* state = (UserDataState*)stateArg;

    // Always rebuild the bundles.
    StartEntry.ClearBundle();
    rebuildBundles(*state);
    return true;
}

void UserDataEffect::ChainNext(BundleEntryRange* chain, BundleEntryRange*)
{
    CacheEffect::ChainNextDefault(chain, StartEntry, EndEntry);
}

void UserDataEffect::GetRange(BundleEntryRange* result)
{
    result->Init(&StartEntry, &EndEntry, Length);
}

CacheEffect* UserDataEffect::Create(TreeCacheNode* node, const State* stateArg, CacheEffect* next)
{
    SF_ASSERT(stateArg->GetType() == State_UserData);
    UserDataState* state = (UserDataState*)stateArg;
    return SF_HEAP_AUTO_NEW(node) UserDataEffect(node, *state, next);
}

void UserDataEffect::rebuildBundles(const UserDataState& state)
{
    UserDataState::Data* userData = 0;
    userData = state.GetUserData();
    StartEntry.Key = SortKey(SortKey_UserDataStart, userData);
}

}} // Scaleform::Render

