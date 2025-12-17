/**************************************************************************

PublicHeader:   Render
Filename    :   Render_CacheEffect.h
Content     :   Defines CacheEffect and CacheEffectChain classes
                attachable to TreeCacheNode for mask, blend mode and
                other effect support.
Created     :   July 8, 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_CacheEffect_H
#define INC_SF_Render_CacheEffect_H

#include "Render_Bundle.h"
#include "Render_States.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {

class TreeCacheNode;


// CacheEffect represents a single effect typically created
// for a specific state, associated with a tree cache node.
// An effect typically contains a Start and End BundleEntry objects,
// that represent rendering commands to be executed before
// and after tree node is rendered, respectively.


class CacheEffect : public NewOverrideBase<StatRender_TreeCache_Mem>
{
    friend class CacheEffectChain;
protected:    
    CacheEffect* pNext;    
    UInt32       Length; // Cumulative number of entries in this chain
                         // and nested one.    
public:
    CacheEffect(CacheEffect* next)
       :  pNext(next), Length(0)
    { }

    virtual ~CacheEffect() { }

    virtual StateType      GetType() const = 0;
    virtual TreeCacheNode* GetSourceNode() const = 0;
    virtual bool           Update(const State*) = 0;

    // Chains next effect bundle to us; fills in result describing us.
    // Input:  *chain is the shape/child bundle list, or was obtained by a
    //          ChainNext call on the following effect.
    // Output: *chain is the resulting chain including us.
    virtual void            ChainNext(BundleEntryRange* chain,
                                      BundleEntryRange* maskChain = 0) = 0;
    // Obtains our bundle range.
    virtual void            GetRange(BundleEntryRange* result) = 0;

    typedef CacheEffect* (*CreateFunc)(TreeCacheNode*, const State*, CacheEffect* next);

protected:
    // A default implementation of ChainNext which inserts the StartEntry before the input chain,
    // the EndEntry after the chain. This is the general behavior of creating an effect chain.
    void ChainNextDefault(BundleEntryRange* chain, BundleEntry& StartEntry, BundleEntry& EndEntry);
};

class MaskEffect;
class FilterEffect;
class ViewMatrix3DEffect;
class ProjectionMatrix3DEffect;

// A chain of cache effects stored within a TreeCacheNode.
// Effects are created based on StateBag entries attached to TreeNode.

class CacheEffectChain
{
    CacheEffect* pEffect;

public:
    CacheEffectChain() : pEffect(0) { }
    ~CacheEffectChain();

    bool IsEmpty() const { return pEffect == 0; }

    // Return 'true' if change took place that resulted in
    // parent bundle chain needing an update.
    // Takes TreeNode change flags.
    bool UpdateEffects(TreeCacheNode* node, unsigned changeFlags);
    
    // Updates the chain of bundles:
    //   - On input *chain should is list on Entrys nested within
    //    an effect set.
    //   - On output, it is the resulting entry set.
    void UpdateBundleChain(BundleEntryRange* chain,
                           BundleEntryRange* maskChain = 0)
    {
        if (pEffect)
            updateBundleChain(pEffect, chain, maskChain);
    }

    bool GetRange(BundleEntryRange* chain)
    {
        if (pEffect)
        {
            pEffect->GetRange(chain);
            return true;
        }
        return false;
    }

    template<class EffectType, StateType Type>
    inline EffectType* GetEffectOfType() const
    {
        CacheEffect* effect = pEffect;
        while (effect && effect->GetType() != Type)
            effect = effect->pNext;
        return (EffectType*)effect;
    }

private:
    void updateBundleChain(CacheEffect* effect,
                           BundleEntryRange* chain,
                           BundleEntryRange* maskChain);
};


//--------------------------------------------------------------------

enum MaskEffectState
{
    MES_NoMask,
    MES_Culled,
    MES_Combinable,
    MES_Entry_Count
};


class MaskEffect : public CacheEffect
{
public:
    MaskEffect(TreeCacheNode* node,
               const MaskEffectState mes, const HMatrix& areaMatrix,
               CacheEffect* next);

    // Mask has three entrys
    BundleEntry     StartEntry, EndEntry, PopEntry;
    MaskEffectState MES;    
    HMatrix         BoundsMatrix;

    HMatrix               GetBoundsMatrix() const { return BoundsMatrix; }
    inline MaskEffectState GetEffectState() const { return MES; }

    // Updates BoundsMatrix and returns 'true' if full update
    // needs to be queued up.
    bool                   UpdateMatrix(const MaskEffectState mes,
                                        const Matrix2F& areaMatrix);

    virtual StateType      GetType() const { return State_MaskNode; }
    virtual TreeCacheNode* GetSourceNode() const { return StartEntry.pSourceNode; }
    virtual bool           Update(const State*);
    virtual void           ChainNext(BundleEntryRange* chain,
                                     BundleEntryRange* maskChain);
    virtual void           GetRange(BundleEntryRange* result);

    static CacheEffect*    Create(TreeCacheNode*, const State*, CacheEffect* next);
};

class BlendModeEffect : public CacheEffect
{
public:
    BlendModeEffect(TreeCacheNode* node, const HMatrix& m, const BlendState& state, CacheEffect* next);

    BundleEntry             StartEntry, EndEntry;
    HMatrix                 BoundsMatrix;

    HMatrix                GetBoundsMatrix() const { return BoundsMatrix; }
    bool                   UpdateMatrix(const Matrix2F& boundsMatrix);
    void                   UpdateCxform(const Cxform& cx);
    virtual StateType      GetType() const { return State_BlendMode; }
    virtual TreeCacheNode* GetSourceNode() const { return StartEntry.pSourceNode; }
    virtual bool           Update(const State*);
    virtual void           ChainNext(BundleEntryRange* chain,
                                     BundleEntryRange*);
    virtual void           GetRange(BundleEntryRange* result);

    static CacheEffect* Create(TreeCacheNode*, const State*, CacheEffect* next);
};

class FilterEffect : public CacheEffect
{
public:
    FilterEffect(TreeCacheNode* node, const HMatrix& m, const FilterState& state, CacheEffect* next);

    bool                    Contributing;
    BundleEntry             StartEntry, EndEntry;
    HMatrix                 BoundsMatrix;

    HMatrix                GetBoundsMatrix() const { return BoundsMatrix; }
    bool                   UpdateMatrix(const Matrix2F& boundsMatrix, const Matrix2F& nodeMatrix, bool forceUncache = false);
    void                   UpdateCxform(const Cxform& cx);
    virtual StateType      GetType() const { return State_Filter; }
    virtual TreeCacheNode* GetSourceNode() const { return StartEntry.pSourceNode; }
    virtual bool           Update(const State*);
    virtual void           ChainNext(BundleEntryRange* chain,
                                     BundleEntryRange*);
    virtual void           GetRange(BundleEntryRange* result);

    static CacheEffect* Create(TreeCacheNode*, const State*, CacheEffect* next);

};

//
// An effect used to apply a different 3D view matrix to a sub-tree.
// User for per-node 3D perspective settings.
// This effect is used in tandem with a ProjectionMatrix3DEffect.
//
class ViewMatrix3DEffect : public CacheEffect
{
public:
    ViewMatrix3DEffect(TreeCacheNode* node, const ViewMatrix3DState& state, CacheEffect* next);

    BundleEntry             StartEntry, EndEntry;

    virtual StateType       GetType() const { return State_ViewMatrix3D; }
    virtual TreeCacheNode*  GetSourceNode() const { return StartEntry.pSourceNode; }
    virtual bool            Update(const State*);
    virtual void            ChainNext(BundleEntryRange* chain, BundleEntryRange*);
    virtual void            GetRange(BundleEntryRange* result);

    static CacheEffect*     Create(TreeCacheNode*, const State*, CacheEffect* next);
};

//
// User for per-node 3D perspective settings.
// An effect used to apply a different 3D perspective matrix to a sub-tree.
// This effect is used in tandem with a ViewMatrix3DEffect.
//
class ProjectionMatrix3DEffect : public CacheEffect
{
public:
    ProjectionMatrix3DEffect(TreeCacheNode* node, const ProjectionMatrix3DState& state, CacheEffect* next);

    BundleEntry             StartEntry, EndEntry;

    virtual StateType       GetType() const { return State_ProjectionMatrix3D; }
    virtual TreeCacheNode*  GetSourceNode() const { return StartEntry.pSourceNode; }
    virtual bool            Update(const State*);
    virtual void            ChainNext(BundleEntryRange* chain, BundleEntryRange*);
    virtual void            GetRange(BundleEntryRange* result);

    static CacheEffect*     Create(TreeCacheNode*, const State*, CacheEffect* next);
};

// UserDataEffect is used for attaching user data to a render node. This is used with the
// setRenderString/setRenderFloat extensions.
class UserDataEffect : public CacheEffect
{
public:
    UserDataEffect(TreeCacheNode* node, const UserDataState& state, CacheEffect* next);

    virtual StateType       GetType() const { return State_UserData; }
    virtual TreeCacheNode*  GetSourceNode() const { return StartEntry.pSourceNode; }
    virtual bool            Update(const State*);
    virtual void            ChainNext(BundleEntryRange* chain, BundleEntryRange*);
    virtual void            GetRange(BundleEntryRange* result);

    static CacheEffect*     Create(TreeCacheNode*, const State*, CacheEffect* next);

protected:
    void                    rebuildBundles(const UserDataState& state);
    BundleEntry             StartEntry, EndEntry;
};


}} // Scaleform::Render

#endif
