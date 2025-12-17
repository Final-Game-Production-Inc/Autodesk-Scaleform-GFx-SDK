/**************************************************************************

Filename    :   Render_TreeCacheText.cpp
Content     :   TreeText Cache implementation
Created     :   2009-2010
Authors     :   Maxim Shemanarev, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_TreeCacheText.h"

#include "Render_PrimitiveBundle.h" // For GetBundle inline
#include "Render_TextMeshProvider.h"
#include "Text/Text_FilterDesc.h"
#include "Renderer2DImpl.h"

#include <stdio.h> // DBG

namespace Scaleform { namespace Render {


//------------------------------------------------------------------------
// ***** TreeCacheText implementation

TreeCacheText::TreeCacheText(TreeText* node, Renderer2DImpl* prenderer2D, unsigned flags)
:   TreeCacheMeshBase(node, SortKey(SortKeyText, (flags&NF_3D)!=0), prenderer2D, flags),
    pNextNoBatch(0), TMProvider(pRenderer2D->GetGlyphCache())
{
}

TreeCacheText::~TreeCacheText()
{
}



void TreeCacheText::UpdateCache(TreeCacheContainer* pparent, UInt16 depth)
{  
    SF_ASSERT(pParent == pparent);
    Depth = depth;
    pRoot = pparent->pRoot;

}

void TreeCacheText::UpdateChildSubtree(const TreeNode::NodeData* data, UInt16 depth)
{
    // We override UpdateChildSubtree so that sortKey can be updated if necessary.
    unsigned newFlags = (GetFlags() & ~NF_PartOfMask) |
                        (GetParent()->GetFlags() & (NF_PartOfMask|NF_PartOfScale9)) |
                        NF_MaskNode_MapTo_PartOfMask(GetFlags());
    if (data && data->Is3D())
        newFlags |= NF_3D;

    if (newFlags != GetFlags())
    {
        SetFlags(newFlags);
    }
    if (data)
        updateMaskCache(data, depth);
}


//------------------------------------------------------------------------
unsigned  TreeCacheText::calcMeshGenFlags() const
{
    unsigned meshGenFlags = 0;
    if (GetFlags() & NF_PartOfMask)
        meshGenFlags |= Mesh_Mask;
    else if ((GetFlags() & NF_EdgeAA_Mask) == NF_EdgeAA_On)        
        meshGenFlags |= Mesh_EdgeAA;        
    if (GetFlags() & NF_PartOfScale9)
        meshGenFlags |= Mesh_Scale9;
    return meshGenFlags;
}

void TreeCacheText::UpdateDistanceFieldUniforms()
{
    if (TMProvider.HasDistanceField())
    {
        // This matrix data isn't used by any of the other fill styles and 
        // only depends on the Filters, so can be shared by all batches in this text field.
        DistFieldUniforms df;
        const Text::TextFilter& tf = GetNodeData()->pDocView->GetFilters();

        if (tf.ShadowParams.BlurX > 0)
        {
            df.ShadowEnable = 1;
            df.ShadowWidth = df.Width * 2 * Alg::Min(3.f, TwipsToPixels(tf.ShadowParams.BlurX));
            df.ShadowOffset[0] = -TwipsToPixels(tf.ShadowParams.Offset.x);
            df.ShadowOffset[1] = -TwipsToPixels(tf.ShadowParams.Offset.y);
            float Offset2 = df.ShadowOffset[0] * df.ShadowOffset[0] + df.ShadowOffset[1] * df.ShadowOffset[1];
            if (Offset2 > 4)
            {
                float Offset = sqrtf(Offset2);
                df.ShadowOffset[0] *= (2.f/Offset);
                df.ShadowOffset[1] *= (2.f/Offset);
            }

            tf.ShadowParams.Colors[0].GetRGBAFloat(df.ShadowColor);
        }
        M.SetUserData(&df, sizeof(df));
    }
}

//------------------------------------------------------------------------
// Updates computed final matrix
void TreeCacheText::UpdateTransform(const TreeNode::NodeData* nodeData,
                                    const TransformArgs& t,
                                    TransformFlags flags)
{
    RectF cullRect(t.CullRect);
    updateCulling(nodeData, t, &cullRect, (TransformFlags) (flags | TF_CullCxform));
    SortParentBounds = nodeData->AproxParentBounds;
    SetFlags(GetFlags() & ~NF_ExpandedBounds );

    // Updates computed final HMatrix
    ComputeFinalMatrix(t, flags);

    const TextLayout* layout = GetNodeData()->pLayout;
    SF_ASSERT(layout);

    // If we were updating Cxform only, skip the rest of the logic, which
    // depends only on matrix. (TBD: Handle Cxform alpha change).
    if (flags & TF_Matrix)
    {
        if (layout->GetParam().TextParam.IsOptRead() &&
            !(M.Has3D() || M.GetMatrix2D().IsFreeRotation()))
        {
            Matrix m2 = M.GetMatrix2D();
            m2.Tx() = floorf(m2.Tx() + ((m2.Tx() < 0) ? -0.5f : +0.5f));
            m2.Ty() = floorf(m2.Ty() + ((m2.Ty() < 0) ? -0.5f : +0.5f));
            M.SetMatrix2D(m2);
        }

        Bundle* bundle = GetBundle();
        if (!bundle)
            return;

        bool updateMesh = false;

        if (!TMProvider.IsCreated())
            return;

        // Making decision. Check to see if it's necessary to rebuild TextMeshProvider.
        // Matrix M.GetMatrix2D/SetMatrix2D: final rendering matrix in screen space

        Matrix4F m4;
        Viewport vp;
        if (M.Has3D() && pRoot)
        {
            getMatrix4F(&m4, t.GetViewProj());
            vp = pRoot->GetNodeData()->VP;
        }

        if (TMProvider.NeedsUpdate(M, m4, vp, layout->GetParam()))
        {
            updateMesh = true;
        }
        // TO DO: Possibly other conditions

        if (updateMesh)
        {
            bundle->UpdateMesh(&SorterShapeNode);
            TMProvider.Clear();
            return;
        }
    }

    if (TMProvider.HasVectorGlyphs())
    {
        unsigned i;
        unsigned meshGenFlags = calcMeshGenFlags();
        for(i = 0; i < TMProvider.GetLayerCount(); ++i)
        {
            TextMeshLayer& layer = TMProvider.GetLayer(i);
            if (layer.Type == TextLayer_Shapes || layer.Type == TextLayer_Shapes_Masked)
            {
                const TextMeshEntry& ent = TMProvider.GetEntry(layer, 0);

                Matrix2F scaling = Matrix2F::Scaling(TMProvider.GetHeightRatio() * layer.SizeScale);
                SF_SIMD_ALIGN( float keyData[MeshKey::MaxKeySize] );
                unsigned keyFlags = TextMeshProvider::CalcVectorParams(layer, ent, scaling, layer.SizeScale, 
                                                                       M, pRenderer2D, meshGenFlags, keyData);

                MeshKeyManager* man = pRenderer2D->GetMeshKeyManager();
                const ToleranceParams& cfg = pRenderer2D->GetToleranceParams();

                if (layer.pMeshKey && layer.pMeshKey->Match(0, flags, keyData, cfg))
                {
                    //printf("M"); // DBG
                }
                else
                {
                    layer.pMeshKey = *man->CreateMatchingKey(layer.pShape, 0, keyFlags , keyData, cfg);

                    if (!layer.pMeshKey->pMesh)
                    {
                        layer.pMeshKey->pMesh = *SF_HEAP_AUTO_NEW(this)
                            Mesh(pRenderer2D, layer.pMeshKey->GetKeySet(), scaling, 0, 0, meshGenFlags);
                    }
                    layer.pMesh = (Mesh*)layer.pMeshKey->pMesh.GetPtr();
                }
            }
        }
    }

    if (flags & TF_Matrix && TMProvider.HasMask())
        TMProvider.UpdateMaskClearBounds(M);
}


//------------------------------------------------------------------------
void TreeCacheText::HandleChanges(unsigned changeBits)
{
    if (changeBits & Change_TextLayout)
    {
        Bundle* bundle = GetBundle();
        if (bundle)
            bundle->UpdateMesh(&SorterShapeNode);

        TMProvider.Clear();
    }

    if (changeBits & Change_EdgeAAMode)
        propagateEdgeAA(CalcEdgeAAMode(GetNodeData(), GetParent()));
}

void TreeCacheText::forceUpdateImages()
{
    HandleChanges(Change_TextLayout);
}

void TreeCacheText::propagateEdgeAA(EdgeAAMode parentEdgeAA)
{
    EdgeAAMode edgeAA = parentEdgeAA;
    if (GetNode())
        edgeAA = TreeCacheNode::CalcEdgeAAMode(GetNodeData(), parentEdgeAA);
    if ((GetFlags() & NF_EdgeAA_Mask) != (unsigned)edgeAA)
    {        
        ApplyFlags(NF_EdgeAA_Mask, edgeAA);
        TMProvider.Clear();
    }
}

void TreeCacheText::propagate3DFlag(unsigned parent3D)
{
    SorterShapeNode.ClearBundle();
    SorterShapeNode.Key = SortKey(SortKeyText, (parent3D&NF_3D)!=0);
    Bundle* bundle = GetBundle();
    if (bundle)
        bundle->UpdateMesh(&SorterShapeNode);
    TMProvider.Clear();
}

void TreeCacheText::propagateMaskFlag(unsigned partOfMask)
{        
    unsigned newFlags = (GetFlags() & ~NF_PartOfMask) | partOfMask;
    if (newFlags != GetFlags())
    {
        SetFlags(newFlags);
    }
}

TextMeshProvider* TreeCacheText::GetMeshProvider()
{
    return TMProvider.IsCreated() ? &TMProvider : 0;
}


void TreeCacheText::getMatrix4F(Matrix4F* m4, const Matrix4F &viewProj) const
{
    m4->MultiplyMatrix(viewProj, M.GetMatrix3D());
}



TextMeshProvider* TreeCacheText::CreateMeshProvider() 
{ 
    SF_ASSERT(!TMProvider.IsCreated());

  //  GlyphCache* cache = pRenderer2D->GetGlyphCache();
  //  pProvider = *SF_HEAP_AUTO_NEW(this) TextMeshProvider(cache);

    const TreeText::NodeData* nodeData = GetNodeData();
    const TextLayout* layout = nodeData->pLayout;

    //SF_ASSERT(layout);
    if (layout && pRoot)
    {
        Matrix4F m4;
        Viewport vp;
        if (M.Has3D())
        {
            Matrix4F viewProj = GetViewProj();
            getMatrix4F(&m4, viewProj);
            vp = pRoot->GetNodeData()->VP;
        }
        //unsigned meshGenFlags = (GetFlags() & NF_PartOfMask) ? Mesh_Mask : Mesh_EdgeAA;
        //if (GetFlags() & NF_PartOfScale9)
        //    meshGenFlags |= Mesh_Scale9;
        TMProvider.CreateMeshData(layout, pRenderer2D, M, m4, vp, calcMeshGenFlags());
    }

    if (TMProvider.IsCreated())
        UpdateDistanceFieldUniforms();

    // Create may have failed.
    return TMProvider.IsCreated() ? &TMProvider : 0;
}




}} // Scaleform::Render
