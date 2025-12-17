/**************************************************************************

Filename    :   Render_TextMeshProvider.cpp
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_TextMeshProvider.h"
#include "Render_TextPrimitiveBundle.h"
#include "Render_GlyphCache.h"
#include "Render_TessCurves.h"
#include "Render_FontCacheHandle.h"
#include "Render_Viewport.h"
#include "Renderer2DImpl.h"

namespace Scaleform { namespace Render {


//------------------------------------------------------------------------
struct ImageGlyphVertex
{
    float   x,y;
    float   u,v;
    void    Set(float x_, float y_, float u_, float v_)
    {
        x = x_;
        y = y_;
        u = u_;
        v = v_;
    }
    static VertexElement Elements[3];
    static VertexFormat  Format;
};

//------------------------------------------------------------------------
VertexElement ImageGlyphVertex::Elements[] =
{
    { 0,  {VET_XY32f} },
    { 8,  {VET_UV32f} },
    { 0,  {VET_None} }
};
VertexFormat ImageGlyphVertex::Format = { sizeof(ImageGlyphVertex), ImageGlyphVertex::Elements };


void VectorGlyphShape::OnEvict(MeshBase *mesh)
{
    SF_UNUSED(mesh);
}

RectF VectorGlyphShape::GetIdentityBounds() const
{
    return Bounds;
}

RectF VectorGlyphShape::GetBounds(const Matrix2F& m) const
{
    SF_UNUSED(m);
    // TO DO: Revise
    return Bounds;
}

RectF VectorGlyphShape::GetCorrectBounds(const Matrix2F& m, float morphRatio, 
                                         StrokeGenerator* gen, const ToleranceParams* tol) const
{
    SF_UNUSED4(m, morphRatio, gen, tol);
    // TO DO: Revise
    return Bounds;
}

bool VectorGlyphShape::HitTestShape(const Matrix2F& m, float x, float y, float morphRatio,
                                    StrokeGenerator* gen, const ToleranceParams* tol) const
{ 
    SF_UNUSED6(m, x, y, morphRatio, gen, tol);
    // TO DO: Revise
    return false;
}

unsigned VectorGlyphShape::GetLayerCount() const
{
    return 1;
}

unsigned VectorGlyphShape::GetFillCount(unsigned layer, unsigned meshGenFlags) const
{
    SF_UNUSED2(layer, meshGenFlags);
    return 1;
}


//------------------------------------------------------------------------
bool VectorGlyphShape::generateNullVectorMesh(VertexOutput* verOut)
{
    VertexXY16iCF32 ver[1];
    UInt16          tri[3];
    ver[0].x = 0; 
    ver[0].y = 0; 
    ver[0].Color = 0; 
    ver[0].Factors[0] = 0;
    ver[0].Factors[1] = 0;
    tri[0] = tri[1] = tri[2] = 0;
    VertexOutput::Fill vfill;
    vfill.VertexCount = 1;
    vfill.IndexCount  = 3;
    vfill.pFormat     = &VertexXY16iCF32::Format; // TO DO: Consider other formats (XY,Color)
    vfill.FillIndex0  = 0;
    vfill.FillIndex1  = 0;
    vfill.MergeFlags  = 0;
    vfill.MeshIndex   = 0;
    if (!verOut->BeginOutput(&vfill, 1))
        return false; 
    verOut->SetVertices(0, 0, ver, 1);
    verOut->SetIndices(0, 0, tri, 3);
    verOut->EndOutput();
    return true;
}

bool VectorGlyphShape::GetData(MeshBase *mesh, VertexOutput* verOut, unsigned meshGenFlags)
{
    SF_ASSERT(pShape);

    Renderer2DImpl* ren = mesh->GetRenderer();
    const Matrix2F& mtx = mesh->GetViewMatrix(); // tbd: Better passed as argument.

    enum { VerBufSize = 128, TriBufSize = 128 };

    VertexXY16iCF32     meshVerC[VerBufSize];
    TessVertex          tessVer[VerBufSize];
    UInt16              tri[TriBufSize * 3];
    TessMesh            tessMesh;

    if (pShape->IsEmpty())
        return generateNullVectorMesh(verOut);

    MeshGenerator* gen = ren->GetMeshGen();

    gen->Clear();

    ToleranceParams param = ren->GetToleranceParams();
    param.CurveTolerance *= 2;
    param.CollinearityTolerance *= 2;

#ifdef SF_RENDER_ENABLE_EDGEAA
    bool edgeAA = ((meshGenFlags & Mesh_EdgeAA) != 0 && (meshGenFlags & Mesh_Mask) == 0);
    gen->mTess.SetEdgeAAWidth(edgeAA ? param.EdgeAAScale * 0.5f : 0.0f);
#endif
    gen->mTess.SetFillRule(Tessellator::FillNonZero);

    ShapePosInfo pos1(pShape->GetStartingPos());
    ShapePathType pathType;
    PathEdgeType pathEdge;
    float coord[Edge_MaxCoord];
    unsigned styles[3];
    bool first = true;

    Matrix2F m2 = mtx;

    while((pathType = pShape->ReadPathInfo(&pos1, coord, styles)) != Shape_EndShape)
    {
        if (!first && pathType == Shape_NewLayer)
            break;
        first = false;

        if (styles[0] != styles[1])
        {
            m2.Transform(&coord[0], &coord[1]);
            gen->mTess.AddVertex(coord[0], coord[1]);
            while((pathEdge = pShape->ReadEdge(&pos1, coord)) != Edge_EndPath)
            {
                if (pathEdge == Edge_LineTo)
                {
                    m2.Transform(&coord[0], &coord[1]);
                    gen->mTess.AddVertex(coord[0], coord[1]);
                }
                else
                if (pathEdge == Edge_QuadTo)
                {
                    m2.Transform(&coord[0], &coord[1]);
                    m2.Transform(&coord[2], &coord[3]);
                    TessellateQuadCurve(&gen->mTess, param, coord[0], coord[1], coord[2], coord[3]);
                }
                else
                if (pathEdge == Edge_CubicTo)
                {
                    m2.Transform(&coord[0], &coord[1]);
                    m2.Transform(&coord[2], &coord[3]);
                    m2.Transform(&coord[4], &coord[5]);
                    TessellateCubicCurve(&gen->mTess, param, coord[0], coord[1], coord[2], coord[3], coord[4], coord[5]);
                }
            }
            gen->mTess.FinalizePath(1, 0, false, false);
        }
        else
        {
            pShape->SkipPathData(&pos1);
        }
    }

    gen->mTess.Tessellate(false);

    bool ret = true;
    if (gen->mTess.GetMeshCount() && gen->mTess.GetVertexCount())
    {
        gen->mTess.Transform(mtx.GetInverse());
        Matrix m16i = gen->mTess.StretchTo(-32768+4, -32768+4, 32768-4, 32768-4);

        VertexOutput::Fill vfill;
        vfill.VertexCount = gen->mTess.GetMeshVertexCount(0);
        vfill.IndexCount  = gen->mTess.GetMeshTriangleCount(0) * 3;
        vfill.pFormat     = &VertexXY16iCF32::Format; // TO DO: Consider other formats (XY,Color)
        vfill.FillIndex0  = 0;
        vfill.FillIndex1  = 0;
        vfill.MergeFlags  = 0;
        vfill.MeshIndex   = 0;

        ret = verOut->BeginOutput(&vfill, 1, m16i);
        if (ret)
        {
            gen->mTess.GetMesh(0, &tessMesh);

            unsigned start = 0;
            unsigned retVer;
            while((retVer = gen->mTess.GetVertices(&tessMesh, tessVer, VerBufSize)) > 0)
            {
                unsigned j;
                for(j = 0; j < retVer; ++j)
                {
                    const TessVertex& tv = tessVer[j];
                    VertexXY16iCF32&  mv = meshVerC[j];
                    mv.x = (SInt16)floorf((tv.x < 0) ? tv.x-0.5f : tv.x+0.5f);
                    mv.y = (SInt16)floorf((tv.y < 0) ? tv.y-0.5f : tv.y+0.5f);
                    mv.Color = 0xFFFFFFFF;
                    mv.Factors[0] = Factors[TessGetAlpha(tv.Flags)];
                    mv.Factors[1] = Factors[TessGetFactor(tv.Flags)];
                }
                verOut->SetVertices(0, start, meshVerC, retVer);
                start += retVer;
            }

            start = 0;
            unsigned nTri = gen->mTess.GetMeshTriangleCount(vfill.MeshIndex);
            while (start < nTri)
            {
                unsigned retTri = TriBufSize;
                if (start + retTri > nTri)
                {
                    retTri = nTri - start;
                    if (retTri == 0) break;
                }
                gen->mTess.GetTrianglesI16(vfill.MeshIndex, tri, start, retTri);
                verOut->SetIndices(0, start*3, tri, retTri*3);
                start += retTri;
            }
            verOut->EndOutput();
        }
    }
    else
        ret = generateNullVectorMesh(verOut);

    gen->Clear();
    return ret;
}


void VectorGlyphShape::GetFillData(FillData* data, unsigned layer, 
                                   unsigned fillIndex, unsigned meshGenFlags)
{
    SF_UNUSED3(layer, fillIndex, meshGenFlags);
    *data = FillData(Fill_VColor);
    data->PrimFill = PrimFill_VColor;
    data->pVFormat = &VertexXY16iCF32::Format;
}

void VectorGlyphShape::GetFillMatrix(MeshBase *mesh, Matrix2F* matrix, unsigned layer,
                                     unsigned fillIndex, unsigned meshGenFlags)
{
    SF_UNUSED4(mesh, layer, fillIndex, meshGenFlags);
    matrix->SetIdentity();
}


















//------------------------------------------------------------------------
TextMeshProvider::TextMeshProvider(GlyphCache* cache) :
    pCache(cache), Flags(0), 
    Notifiers(cache->GetHeap()),
    Entries(cache->GetHeap()),
    Layers(cache->GetHeap()),
    PinCount(0), pBundle(0), pBundleEntry(0),
    HeightRatio(0), ClipBox(0,0,0,0), Bounds(0,0,0,0), pRenderer(0)
{}

//------------------------------------------------------------------------
TextMeshProvider::~TextMeshProvider()
{  
    Clear();
}

//------------------------------------------------------------------------
void TextMeshProvider::ClearEntries()
{
    UPInt i;
    for(i = 0; i < Notifiers.GetSize(); ++i)
    {
        pCache->RemoveNotifier(Notifiers[i]);
    }
    Notifiers.ClearAndRelease();
    for(i = 0; i < Entries.GetSize(); ++i)
    {
        TextMeshEntry& ent = Entries[i];
        if (ent.LayerType == TextLayer_Shapes ||
            ent.LayerType == TextLayer_Shapes_Masked)
        {
            ent.EntryData.VectorData.pFont->Release();
        }
    }
    Entries.ClearAndRelease();
    Flags &= ~(BF_Created | BF_HasVectorGlyphs | BF_HasUnderlines | BF_HasMask);
}

//------------------------------------------------------------------------
void TextMeshProvider::Clear()
{
    //SF_ASSERT(!IsInList()); // Strict verification

    // Permissive verification
    //---------------------
    if (IsInList())
    {
        SetInUseList(false);
        if (IsInPinList())
        {
            SetInPinList(false);
            UnpinSlots();
        }
        RemoveNode();
    }
    //---------------------

    if (pBundle)
    {
        pBundle->removeEntryFromLayers(pBundleEntry);
        ClearBundle();
    }

    ClearEntries();
    Entries.ClearAndRelease();
    Layers.ClearAndRelease();
}


//------------------------------------------------------------------------
// Call-back from Renderer that performs a "delayed pin" of the batch
void TextMeshProvider::AddToInUseList()
{
    //SF_ASSERT(!IsInList()); // Strict verification

    if (Flags & (BF_InUseList | BF_InPinList)) // Permissive verification 
        return;    
    SetInUseList(true);
    pCache->AddToInUseList(this);
}

//------------------------------------------------------------------------
// Returns the latest fence used by any meshes in any layers of the provider.
Fence* TextMeshProvider::GetLatestFence() const
{
    UPInt layerIndex;
    Ptr<Fence> latestFence = 0;
    for(layerIndex = 0; layerIndex < Layers.GetSize(); ++layerIndex)
    {
        const TextMeshLayer& layer = Layers[layerIndex];
        Mesh* pmesh = layer.pMesh;
        if ( !pmesh )
            continue;
        for ( UPInt cacheItem = 0; cacheItem < pmesh->CacheItems.GetSize(); ++cacheItem )
        {
            if ( !pmesh->CacheItems[cacheItem]->GPUFence )
                continue;
            if (!latestFence || *pmesh->CacheItems[cacheItem]->GPUFence > *latestFence)
                latestFence = pmesh->CacheItems[cacheItem]->GPUFence;
        }
    }
    return latestFence;
}


// GlyphQueue interaction functions
//
//------------------------------------------------------------------------
// Call-back from GlyphQueue, when some glyph in the batch is evicted and released.
void TextMeshProvider::OnEvictSlots()
{
    SF_ASSERT(!IsInList());
    SF_ASSERT((Flags & BF_Creating) == 0);

    //if (Creating)
    //{
    //    ClearEntries();
    //    if (pBundle)
    //    {
    //        pBundle->removeEntryFromLayers(pBundleEntry);
    //        ClearBundle();
    //    }
    //}
    //else
    {
        Clear();
    }
}

//------------------------------------------------------------------------
void TextMeshProvider::AddNotifier(TextNotifier* notifier)
{
    if (notifier)
    {
        Notifiers.PushBack(notifier);
        pCache->PinSlot(notifier->pSlot);
    }
}

//------------------------------------------------------------------------
void TextMeshProvider::PinSlots()
{
    SF_ASSERT(!IsInList());
    for(UPInt i = 0; i < Notifiers.GetSize(); ++i)
    {
        pCache->PinSlot(Notifiers[i]->pSlot);
    }
}

//------------------------------------------------------------------------
void TextMeshProvider::UnpinSlots()
{
    SF_ASSERT(!IsInList());

    Ptr<Fence> fence = GetLatestFence();
    for(UPInt i = 0; i < Notifiers.GetSize(); ++i)
    {
        pCache->UnpinSlot(Notifiers[i]->pSlot, fence);
    }
}



//------------------------------------------------------------------------
MeshUseStatus TextMeshProvider::GetMeshUseStatus() const
{
    MeshUseStatus status = MUS_Uncached;
    if (PinCount)
        return MUS_Pinned;
    for (UPInt i=0; i< GetLayerCount(); i++)
    {
        const TextMeshLayer& l = GetLayer((unsigned)i);
        if (l.Type > TextLayer_RasterText)
            break;
        if ((l.Type >= TextLayer_Shadow) && l.pMesh)
        {
            MeshUseStatus s = l.pMesh->GetUseStatus();
            if (s > status)
                status = s;
        }
    }
    return status;
}



//------------------------------------------------------------------------
void TextMeshProvider::setMeshData(TessBase* tess, VertexOutput* verOut, const UInt32* colors, VertexCountType& verCount)
{
    enum { ChunkSize = 64 };
    TessVertex tessVer[ChunkSize];
    VertexXY16iCF32 vertices[ChunkSize];
    UInt16 tri[ChunkSize * 3];

    TessMesh tessMesh;
    tess->GetMesh(0, &tessMesh);

    unsigned retVer;
    while((retVer = tess->GetVertices(&tessMesh, tessVer, ChunkSize)) > 0)
    {
        unsigned j;
        for(j = 0; j < retVer; ++j)
        {
            const TessVertex& tv = tessVer[j];
            VertexXY16iCF32&  mv = vertices[j];
            mv.x = (SInt16)floorf((tv.x < 0) ? tv.x-0.5f : tv.x+0.5f);
            mv.y = (SInt16)floorf((tv.y < 0) ? tv.y-0.5f : tv.y+0.5f);

            UInt32 c1, c2;
            if (TessStyleMixesColors(tv.Flags))
            {
                c1 = colors[tv.Styles[0]-1];
                c2 = colors[tv.Styles[1]-1];
                mv.Color = ((c1 & 0xFEFEFEFE) >> 1) | ((c2 & 0xFEFEFEFE) >> 1);
            }
            else
            {
                mv.Color = colors[tv.Styles[TessGetUsedStyle(tv.Flags)]-1];
            }
            mv.Factors[0] = Factors[TessGetAlpha(tv.Flags)];
            mv.Factors[1] = Factors[TessGetFactor(tv.Flags)];
        }
        verOut->SetVertices(0, verCount.VStart, vertices, retVer);
        verCount.VStart += retVer;
    }

    unsigned start = 0;
    unsigned nTri = tess->GetMeshTriangleCount(0);
    while (start < nTri)
    {
        unsigned retTri = ChunkSize;
        if (start + retTri > nTri)
        {
            retTri = nTri - start;
            if (retTri == 0) break;
        }
        tess->GetTrianglesI16(0, tri, start, retTri);
        verOut->SetIndices(0, verCount.IStart*3, tri, retTri*3);
        start += retTri;
        verCount.IStart += retTri;
    }
}


//------------------------------------------------------------------------
bool TextMeshProvider::generateSelection(Renderer2DImpl* ren, VertexOutput* verOut, 
                                         const TextMeshLayer& layer, const Matrix2F& mtx,
                                         unsigned meshGenFlags)
{
    MeshGenerator* gen = ren->GetMeshGen();
    gen->Clear();
    gen->mTess.SetFillRule(Tessellator::FillNonZero);
    gen->mTess.SetToleranceParam(ren->GetToleranceParams());

    ArrayStaticBuffPOD<UInt32, 16> colors(Memory::GetGlobalHeap());

    const ToleranceParams param = ren->GetToleranceParams();
#ifdef SF_RENDER_ENABLE_EDGEAA
    bool edgeAA = ((meshGenFlags & Mesh_EdgeAA) != 0 && (meshGenFlags & Mesh_Mask) == 0);
    gen->mTess.SetEdgeAAWidth(edgeAA ? param.EdgeAAScale * 0.5f : 0.0f);
#endif
    for(unsigned i = 0; i < layer.Count; ++i)
    {
        const TextMeshEntry& ent = Entries[i+layer.Start];
        const SelectionEnt& sel = ent.EntryData.SelectionData;
        RectF r(sel.Coord[0], sel.Coord[1], sel.Coord[2], sel.Coord[3]);

        if (Flags & BF_Clip)
            r.Intersect(ClipBox);
        else
            r.Intersect(Bounds); // still need to clip selection using Bounds

        if (!r.IsEmpty())
        {
            float x, y;
            x = r.x1; y = r.y1; mtx.Transform(&x, &y); gen->mTess.AddVertex(x, y);
            x = r.x2; y = r.y1; mtx.Transform(&x, &y); gen->mTess.AddVertex(x, y);
            x = r.x2; y = r.y2; mtx.Transform(&x, &y); gen->mTess.AddVertex(x, y);
            x = r.x1; y = r.y2; mtx.Transform(&x, &y); gen->mTess.AddVertex(x, y);
        }
        colors.PushBack(ent.mColor);
        gen->mTess.ClosePath();
        gen->mTess.FinalizePath(0, i+1, false, false);
    }

    gen->mTess.Tessellate(false);

    bool ret = true;
    if (gen->mTess.GetMeshCount() && gen->mTess.GetVertexCount())
    {
        gen->mTess.Transform(mtx.GetInverse());
        Matrix m16i = gen->mTess.StretchTo(-32768+4, -32768+4, 32768-4, 32768-4);

        VertexOutput::Fill vfill = { gen->mTess.GetMeshVertexCount(0), 
                                     gen->mTess.GetMeshTriangleCount(0) * 3, 
                                     &VertexXY16iCF32::Format, 0, 0, 0 };

        ret = verOut->BeginOutput(&vfill, 1, m16i);
        if (ret)
        {
            VertexCountType verCount;
            setMeshData(&gen->mTess, verOut, &colors[0], verCount);
            verOut->EndOutput();
        }
    }
    else
       ret = generateNullVectorMesh(verOut);

    gen->Clear();
    return ret;
}


struct CornerVertex
{
    float x, y;
    CornerVertex() {}
    CornerVertex(float x_, float y_) : x(x_), y(y_) {}
};

//------------------------------------------------------------------------
template<class VertexType> 
static void calcMiter(const VertexType v0, const VertexType v1, const VertexType v2, 
                      float width, float* x, float* y)
{
    float len1 = Math2D::Distance(v0, v1);
    float len2 = Math2D::Distance(v1, v2);
    float dx1 = width * (v1.y - v0.y) / len1;
    float dy1 = width * (v0.x - v1.x) / len1;
    float dx2 = width * (v2.y - v1.y) / len2;
    float dy2 = width * (v1.x - v2.x) / len2;
    *x  = v1.x;
    *y  = v1.y;
    Math2D::Intersection(v0.x+dx1, v0.y+dy1, v1.x+dx1, v1.y+dy1,
                         v1.x+dx2, v1.y+dy2, v2.x+dx2, v2.y+dy2,
                         x, y, 0.001f);
}



//------------------------------------------------------------------------
bool TextMeshProvider::generateRectangle(Renderer2DImpl* ren, VertexOutput* verOut, 
                                         const Matrix2F& mtx, const float* coord, 
                                         UInt32 fillColor, UInt32 borderColor, 
                                         unsigned meshGenFlags)
{
    UInt32 colors[2] = { fillColor, borderColor };
    MeshGenerator* gen = ren->GetMeshGen();
    gen->Clear();
    gen->mTess.SetFillRule(Tessellator::FillNonZero);
    gen->mTess.SetToleranceParam(ren->GetToleranceParams());

    const ToleranceParams param = ren->GetToleranceParams();
#ifdef SF_RENDER_ENABLE_EDGEAA
    bool edgeAA = ((meshGenFlags & Mesh_EdgeAA) != 0 && (meshGenFlags & Mesh_Mask) == 0);
    gen->mTess.SetEdgeAAWidth(edgeAA ? param.EdgeAAScale * 0.5f : 0.0f);
#endif
    float coordTr[8] = 
    {
        coord[0], coord[1],
        coord[2], coord[1],
        coord[2], coord[3],
        coord[0], coord[3]
    };
    mtx.Transform(coordTr+0, coordTr+1);
    mtx.Transform(coordTr+2, coordTr+3);
    mtx.Transform(coordTr+4, coordTr+5);
    mtx.Transform(coordTr+6, coordTr+7);

    if (fillColor)
    {
        gen->mTess.AddVertex(coordTr[0], coordTr[1]);
        gen->mTess.AddVertex(coordTr[2], coordTr[3]);
        gen->mTess.AddVertex(coordTr[4], coordTr[5]);
        gen->mTess.AddVertex(coordTr[6], coordTr[7]);
        gen->mTess.ClosePath();
        gen->mTess.FinalizePath(0, 1, false, false);
    }

    if (borderColor)
    {
        gen->mTess.AddVertex(coordTr[0], coordTr[1]);
        gen->mTess.AddVertex(coordTr[2], coordTr[3]);
        gen->mTess.AddVertex(coordTr[4], coordTr[5]);
        gen->mTess.AddVertex(coordTr[6], coordTr[7]);
        gen->mTess.ClosePath();
        gen->mTess.FinalizePath(0, 2, false, false);

        float x, y;
        CornerVertex v1(coordTr[0], coordTr[1]);
        CornerVertex v2(coordTr[2], coordTr[3]);
        CornerVertex v3(coordTr[4], coordTr[5]);
        CornerVertex v4(coordTr[6], coordTr[7]);
        calcMiter(v2, v1, v4, 1, &x, &y); gen->mTess.AddVertex(x, y);
        calcMiter(v1, v4, v3, 1, &x, &y); gen->mTess.AddVertex(x, y);
        calcMiter(v4, v3, v2, 1, &x, &y); gen->mTess.AddVertex(x, y);
        calcMiter(v3, v2, v1, 1, &x, &y); gen->mTess.AddVertex(x, y);
        gen->mTess.ClosePath();
        gen->mTess.FinalizePath(0, 2, false, false);
    }

    gen->mTess.Tessellate(false);
    gen->mTess.Transform(mtx.GetInverse());
    Matrix m16i = gen->mTess.StretchTo(-32768+4, -32768+4, 32768-4, 32768-4);

    VertexOutput::Fill vfill = { gen->mTess.GetMeshVertexCount(0), 
                                 gen->mTess.GetMeshTriangleCount(0) * 3, 
                                 &VertexXY16iCF32::Format, 0, 0, 0 };

    if (vfill.VertexCount == 0)
        return generateNullVectorMesh(verOut);

    bool ret = verOut->BeginOutput(&vfill, 1, m16i);
    if (ret)
    {
        VertexCountType verCount;
        setMeshData(&gen->mTess, verOut, colors, verCount);
        verOut->EndOutput();
    }
    gen->Clear();
    return ret;
}


//------------------------------------------------------------------------
bool TextMeshProvider::clipGlyphRect(RectF& chr, RectF& tex) const
{
    if (Flags & BF_Clip)
    {
        RectF clippedChr = chr;
        clippedChr.Intersect(ClipBox);
        if (clippedChr.IsEmpty())
        {
            chr = ClipBox;
            chr.x2 = chr.x1; chr.y2 = chr.y1;
            tex.x2 = tex.x1; tex.y2 = tex.y1;
            return false;
        }

        if (clippedChr != chr)
        {
            RectF clippedTex = tex;

            if (chr.x1 != clippedChr.x1)
                clippedTex.x1 = tex.x1 + (clippedChr.x1 - chr.x1) * tex.Width() / chr.Width();

            if (chr.y1 != clippedChr.y1)
                clippedTex.y1 = tex.y1 + (clippedChr.y1 - chr.y1) * tex.Height() / chr.Height();

            if (chr.x2 != clippedChr.x2)
                clippedTex.x2 = tex.x2 - (chr.x2 - clippedChr.x2) * tex.Width() / chr.Width();

            if (chr.y2 != clippedChr.y2)
                clippedTex.y2 = tex.y2 - (chr.y2 - clippedChr.y2) * tex.Height() / chr.Height();

            chr = clippedChr;
            tex = clippedTex;
        }
    }
    return true;
}


//------------------------------------------------------------------------
bool TextMeshProvider::generateRasterMesh(VertexOutput* verOut, const TextMeshLayer& layer)
{
    enum { GlyphChunkSize = 64 };

    RasterGlyphVertex vertices[GlyphChunkSize * 4];
    UInt16 indices[GlyphChunkSize * 6];

    unsigned totalGlyphs = layer.Count;
    VertexOutput::Fill vfill = { 4 * totalGlyphs, 6 * totalGlyphs, &RasterGlyphVertex::Format, 0, 0, 0 };
    float scaleU = pCache->GetScaleU();
    float scaleV = pCache->GetScaleV();

    if (!verOut->BeginOutput(&vfill, 1))
        return false;

    unsigned i = 0;
    unsigned glyphCount = 0;
    unsigned glyphOffset = 0;
    for(i = 0; i < totalGlyphs; ++i)
    {
        if(glyphCount >= GlyphChunkSize)
        {
            verOut->SetVertices(0, glyphOffset*4, vertices, GlyphChunkSize * 4);
            verOut->SetIndices(0, glyphOffset*6, indices, GlyphChunkSize * 6);
            glyphCount = 0;
            glyphOffset += GlyphChunkSize;
        }
        const TextMeshEntry& ent = Entries[i+layer.Start];
        const GlyphRasterEnt& ras = ent.EntryData.RasterData;
        const GlyphRect& r = ras.pGlyph->mRect;
        UInt32 c = ent.mColor;

        unsigned verIdx = glyphCount*4;
        unsigned indIdx = glyphCount*6;
        unsigned idx = i*4;
        RectF chr(ras.Coord[0], ras.Coord[1], ras.Coord[2], ras.Coord[3]);
        float texX1 = (r.x + 1) * scaleU;
        float texY1 = (r.y + 1) * scaleV;
        float texX2 = texX1 + (r.w - 2) * scaleU;
        float texY2 = texY1 + (r.h - 2) * scaleV;
        RectF tex(texX1, texY1, texX2, texY2);

        clipGlyphRect(chr, tex);

        vertices[verIdx+0].Set(chr.x1, chr.y1, tex.x1, tex.y1, c);
        vertices[verIdx+1].Set(chr.x2, chr.y1, tex.x2, tex.y1, c);

        vertices[verIdx+2].Set(chr.x2, chr.y2, tex.x2, tex.y2, c);
        vertices[verIdx+3].Set(chr.x1, chr.y2, tex.x1, tex.y2, c);
        indices[indIdx+0] = UInt16(idx+0);
        indices[indIdx+1] = UInt16(idx+1);
        indices[indIdx+2] = UInt16(idx+2);
        indices[indIdx+3] = UInt16(idx+2);
        indices[indIdx+4] = UInt16(idx+3);
        indices[indIdx+5] = UInt16(idx+0);
        ++glyphCount;
    }

    if(glyphCount)
    {
        verOut->SetVertices(0, glyphOffset*4, vertices, glyphCount * 4);
        verOut->SetIndices(0, glyphOffset*6, indices, glyphCount * 6);
    }
    else
    {
        return generateNullVectorMesh(verOut);
    }

    verOut->EndOutput();
    return true;
}


//------------------------------------------------------------------------
bool TextMeshProvider::generatePackedMesh(VertexOutput* verOut, const TextMeshLayer& layer)
{
    enum { GlyphChunkSize = 64 };

    RasterGlyphVertex verA8[GlyphChunkSize * 4];
    ImageGlyphVertex  verC32[GlyphChunkSize * 4];
    UInt16 indices[GlyphChunkSize * 6];

    unsigned totalGlyphs = layer.Count;
    VertexOutput::Fill vfillA8 = { 4 * totalGlyphs, 6 * totalGlyphs, &RasterGlyphVertex::Format, 0, 0, 0 };
    VertexOutput::Fill vfillC32 = { 4 * totalGlyphs, 6 * totalGlyphs, &ImageGlyphVertex::Format, 0, 0, 0 };

    bool a8 = Entries[layer.Start].EntryData.PackedData.pGlyph->pImage->GetFormat() == Image_A8;

    if (!verOut->BeginOutput(a8 ? &vfillA8 : &vfillC32, 1))
        return false;

    unsigned i = 0;
    unsigned glyphCount = 0;
    unsigned glyphOffset = 0;
    for(i = 0; i < totalGlyphs; ++i)
    {
        if(glyphCount >= GlyphChunkSize)
        {
            verOut->SetVertices(0, glyphOffset*4, a8 ? (void*)verA8 : (void*)verC32, GlyphChunkSize * 4);
            verOut->SetIndices(0, glyphOffset*6, indices, GlyphChunkSize * 6);
            glyphCount = 0;
            glyphOffset += GlyphChunkSize;
        }
        const TextMeshEntry& ent = Entries[i+layer.Start];
        const GlyphTextureEnt& pak = ent.EntryData.PackedData;
        RectF tex = pak.pGlyph->UvBounds;
        UInt32 c = ent.mColor;

        unsigned verIdx = glyphCount*4;
        unsigned indIdx = glyphCount*6;
        unsigned idx = i*4;
        RectF chr(pak.Coord[0], pak.Coord[1], pak.Coord[2], pak.Coord[3]);

        clipGlyphRect(chr, tex);

        if (a8)
        {
            verA8[verIdx+0].Set(chr.x1, chr.y1, tex.x1, tex.y1, c);
            verA8[verIdx+1].Set(chr.x2, chr.y1, tex.x2, tex.y1, c);
            verA8[verIdx+2].Set(chr.x2, chr.y2, tex.x2, tex.y2, c);
            verA8[verIdx+3].Set(chr.x1, chr.y2, tex.x1, tex.y2, c);
        }
        else
        {
            verC32[verIdx+0].Set(chr.x1, chr.y1, tex.x1, tex.y1);
            verC32[verIdx+1].Set(chr.x2, chr.y1, tex.x2, tex.y1);
            verC32[verIdx+2].Set(chr.x2, chr.y2, tex.x2, tex.y2);
            verC32[verIdx+3].Set(chr.x1, chr.y2, tex.x1, tex.y2);
        }
        indices[indIdx+0] = UInt16(idx+0);
        indices[indIdx+1] = UInt16(idx+1);
        indices[indIdx+2] = UInt16(idx+2);
        indices[indIdx+3] = UInt16(idx+2);
        indices[indIdx+4] = UInt16(idx+3);
        indices[indIdx+5] = UInt16(idx+0);
        ++glyphCount;
    }

    if(glyphCount)
    {
        verOut->SetVertices(0, glyphOffset*4, a8 ? (void*)verA8 : (void*)verC32, glyphCount * 4);
        verOut->SetIndices(0, glyphOffset*6, indices, glyphCount * 6);
    }
    else
    {
        generateNullVectorMesh(verOut);
    }

    verOut->EndOutput();
    return true;
}


//------------------------------------------------------------------------
bool TextMeshProvider::generateImageMesh(VertexOutput* verOut, const TextMeshLayer& layer)
{
    ImageGlyphVertex ver[4];
    UInt16 idx[6] = {0,1,2,0,2,3};
    VertexOutput::Fill vfill = { 4, 6, &ImageGlyphVertex::Format, 0, 0, 0 };
    const TextImageEnt& ent = Entries[layer.Start].EntryData.ImageData;
    Image* img = ent.pImage;

    // Get coordinate->UV transformation matrix, which does divide by image size,
    // scaling and sub-image adjustment.
    Matrix2F uvGenMatrix(Matrix2F::NoInit);
    img->GetUVNormMatrix(&uvGenMatrix, pCache->GetTextureManager());

    float x0 = ent.Coord[0];
    float y0 = ent.Coord[1];

    RectF chr(0, 0, ent.Coord[2] - x0, ent.Coord[3] - y0);
    uvGenMatrix.Transform(&chr.x1, &chr.y1);
    uvGenMatrix.Transform(&chr.x2, &chr.y2);
    chr += PointF(x0, y0);

    RectF tex(0, 0, 1, 1);
    uvGenMatrix.Transform(&tex.x1, &tex.y1);
    uvGenMatrix.Transform(&tex.x2, &tex.y2);

    clipGlyphRect(chr, tex);

    ver[0].Set(chr.x1, chr.y1, tex.x1, tex.y1);
    ver[1].Set(chr.x2, chr.y1, tex.x2, tex.y1);
    ver[2].Set(chr.x2, chr.y2, tex.x2, tex.y2);
    ver[3].Set(chr.x1, chr.y2, tex.x1, tex.y2);

    if (!verOut->BeginOutput(&vfill, 1))
        return false;

    verOut->SetVertices(0, 0, ver, 4);
    verOut->SetIndices(0, 0, idx, 6);

    verOut->EndOutput();
    return true;
}

//------------------------------------------------------------------------
bool TextMeshProvider::generateUnderlines(Renderer2DImpl* ren, VertexOutput* verOut, 
                                          const TextMeshLayer& layer, const Matrix2F& mtx, 
                                          unsigned meshGenFlags)
{
    ArrayStaticBuffPOD<UInt32, 16> colors(Memory::GetGlobalHeap());
    MeshGenerator* gen = ren->GetMeshGen();
    gen->Clear();

    bool edgeAA = ((meshGenFlags & Mesh_EdgeAA) != 0 && (meshGenFlags & Mesh_Mask) == 0);

    gen->mStrokerAA.SetLineCap(StrokerTypes::ButtCap);
    gen->mStrokerAA.SetLineJoin(StrokerTypes::MiterJoin);
    if (edgeAA)
    {
        gen->mStrokerAA.SetSolidWidth(0.0f);
        gen->mStrokerAA.SetEdgeAAWidth(0.48f);
    }
    else
    {
        gen->mStrokerAA.SetSolidWidth(1.0f);
        gen->mStrokerAA.SetEdgeAAWidth(0.0f);
    }

    unsigned i;
    for (i = 0; i < layer.Count; ++i)
    {
        const TextMeshEntry& ent = Entries[i+layer.Start];
        const UnderlineEnt& und = ent.EntryData.UnderlineData;

        if (edgeAA)
        {
            if (und.Style == TextUnderline_Thick || und.Style == TextUnderline_DottedThick)
                gen->mStrokerAA.SetSolidWidth(2.0f);
            else
                gen->mStrokerAA.SetSolidWidth(0.0f);

            if (und.Style == TextUnderline_DitheredThick)
                gen->mStrokerAA.SetSolidWidth(0.5f);

            if (und.Style == TextUnderline_Dithered)
                gen->mStrokerAA.SetEdgeAAWidth(0.25f);
            else
                gen->mStrokerAA.SetEdgeAAWidth(0.48f);
        }
        else
        {
            if (und.Style == TextUnderline_Thick || und.Style == TextUnderline_DottedThick)
                gen->mStrokerAA.SetSolidWidth(2.0f);
            else
                gen->mStrokerAA.SetSolidWidth(1.0f);

            if (und.Style == TextUnderline_DitheredThick)
                gen->mStrokerAA.SetSolidWidth(1.5f);

            gen->mStrokerAA.SetEdgeAAWidth(0.0f);
        }

        gen->mStrokerAA.SetStyleLeft(i+1);
        gen->mStrokerAA.SetStyleRight(i+1);
        colors.PushBack(ent.mColor);

        float x1 = und.x;
        float y1 = und.y;
        float x2 = und.x + und.Len;
        float y2 = y1;

        mtx.Transform(&x1, &y1);
        mtx.Transform(&x2, &y2);

        if (und.Style == TextUnderline_Single || und.Style == TextUnderline_Thick)
        {
            gen->mStrokerAA.AddVertex(x1, floorf(y1)+0.5f);
            gen->mStrokerAA.AddVertex(x2, floorf(y2)+0.5f);
            gen->mStrokerAA.FinalizePath(0, 0, false, false);
        }

        float x;
        if (und.Style == TextUnderline_Dotted || und.Style == TextUnderline_DottedThick)
        {
            x1 = (float)floor(x1)-0.5f;
            x2 = (float)floor(x2)+0.5f;
            y1 = (float)floor(y1)+0.5f;
            y2 = (float)floor(y2)+0.5f;
            for (x = x1; x <= x2; x += 5)
            {
                gen->mStrokerAA.AddVertex(x,   floorf(y1)+0.5f);
                gen->mStrokerAA.AddVertex(x+2, floorf(y2)+0.5f);
                gen->mStrokerAA.FinalizePath(0, 0, false, false);
            }
        }

        if (und.Style == TextUnderline_Dithered || und.Style == TextUnderline_DitheredThick)
        {
            x1 = (float)floor(x1);
            x2 = (float)floor(x2)+1;
            y1 = (float)floor(y1)+0.5f;
            y2 = (float)floor(y2)+0.5f;
            float dx = 4;
            float dy = 0.75f;
            if (und.Style == TextUnderline_DitheredThick)
            {
                dx = 6;
                dy = 1.25f;
            }
            for (x = x1; x <= x2; x += dx)
            {
                gen->mStrokerAA.AddVertex(x,      floorf(y1)+dy);
                gen->mStrokerAA.AddVertex(x+dx/2, floorf(y2)-dy);
            }
            gen->mStrokerAA.FinalizePath(0, 0, false, false);
        }

    }

    bool ret = true;
    if (gen->mStrokerAA.GetVertexCount())
    {
        gen->mStrokerAA.Transform(mtx.GetInverse());
        Matrix m16i = gen->mStrokerAA.StretchTo(-32768+4, -32768+4, 32768-4, 32768-4);

        VertexOutput::Fill vfill = { gen->mStrokerAA.GetMeshVertexCount(0), 
                                     gen->mStrokerAA.GetMeshTriangleCount(0) * 3, 
                                     &VertexXY16iCF32::Format, 0, 0, 0 };

        ret = verOut->BeginOutput(&vfill, 1, m16i);
        if (ret)
        {
            VertexCountType verCount;
            setMeshData(&gen->mStrokerAA, verOut, &colors[0], verCount);
            verOut->EndOutput();
        }
    }
    else
        ret = generateNullVectorMesh(verOut);

    gen->mStrokerAA.SetStyleLeft(1);
    gen->mStrokerAA.SetStyleRight(1);
    gen->Clear();
    return true;
}


//------------------------------------------------------------------------
HMatrix TextMeshProvider::UpdateMaskClearBounds(HMatrix viewMat)
{ 
    SF_ASSERT(pRenderer);
    Matrix2F m2d;
    m2d.SetRectToRect(0, 0, 1, 1, ClearBox.x1, ClearBox.y1, ClearBox.x2, ClearBox.y2);

    if (viewMat.Has3D())
    {
        Matrix3F m3d = viewMat.GetMatrix3D();
        m3d.Prepend(m2d);
        if (ClearBounds.IsNull())
            ClearBounds = pRenderer->GetMatrixPool().CreateMatrix(m3d);
        else
            ClearBounds.SetMatrix3D(m3d);
    }
    else
    {
        m2d.Append(viewMat.GetMatrix2D());
        if (ClearBounds.IsNull())
            ClearBounds = pRenderer->GetMatrixPool().CreateMatrix(m2d);
        else
            ClearBounds.SetMatrix2D(m2d);
    }
    return ClearBounds;
}


//------------------------------------------------------------------------
HMatrix TextMeshProvider::GetMaskClearBounds() const
{ 
    SF_ASSERT(!ClearBounds.IsNull());
    return ClearBounds;
}

//------------------------------------------------------------------------
bool TextMeshProvider::generateMask(VertexOutput* verOut, const TextMeshLayer& layer)
{
    const MaskEnt& e = Entries[layer.Start].EntryData.MaskData;
    SInt16 coord[4] = { -32768+4, -32768+4, 32768-4, 32768-4 };

    Matrix2F m16i;
    m16i.SetRectToRect(coord[0], coord[1], coord[2], coord[3], e.Coord[0], e.Coord[1], e.Coord[2], e.Coord[3]);

    VertexXY16i ver[4];
    ver[0].x = coord[0];
    ver[0].y = coord[1];
    ver[1].x = coord[2];
    ver[1].y = coord[1];
    ver[2].x = coord[2];
    ver[2].y = coord[3];
    ver[3].x = coord[0];
    ver[3].y = coord[3];

    UInt16 idx[6] = {0,1,2,0,2,3};

    VertexOutput::Fill vfill = { 4, 6, &VertexXY16i::Format, 0, 0, 0 };
    if (!verOut->BeginOutput(&vfill, 1, m16i))
        return false;

    verOut->SetVertices(0, 0, ver, 4);
    verOut->SetIndices(0, 0, idx, 6);

    verOut->EndOutput();
    return true;
}

//------------------------------------------------------------------------
bool TextMeshProvider::generateNullVectorMesh(VertexOutput* verOut)
{
    VertexXY16iCF32 ver[1];
    UInt16          tri[3];
    ver[0].x = 0; 
    ver[0].y = 0; 
    ver[0].Color = 0; 
    ver[0].Factors[0] = 0;
    ver[0].Factors[1] = 0;
    tri[0] = tri[1] = tri[2] = 0;
    VertexOutput::Fill vfill;
    vfill.VertexCount = 1;
    vfill.IndexCount  = 3;
    vfill.pFormat     = &VertexXY16iCF32::Format; // TO DO: Consider other formats (XY,Color)
    vfill.FillIndex0  = 0;
    vfill.FillIndex1  = 0;
    vfill.MergeFlags  = 0;
    vfill.MeshIndex   = 0;
    if (!verOut->BeginOutput(&vfill, 1))
        return false; 
    verOut->SetVertices(0, 0, ver, 1);
    verOut->SetIndices(0, 0, tri, 3);
    verOut->EndOutput();
    return true;
}


//------------------------------------------------------------------------
bool TextMeshProvider::GetData(MeshBase *mesh, VertexOutput* verOut, unsigned meshGenFlags)
{
    unsigned        layerIdx = mesh->GetLayer();
    Renderer2DImpl* ren = mesh->GetRenderer();
    const TextMeshLayer& layer = GetLayer(layerIdx);
    TextMeshEntry*  ent = &Entries[layer.Start];

    // Deduce the tessellation matrix from just the height ratio (local-to-screen scale).
    // It results in some degradation of EdgeAA with heavy non-uniform transformations,
    // but prevents from having degenerate matrices. It's important for 3D to eliminate  
    // visual EdgeAA artifacts.
    Matrix2F mtx = Matrix2F::Scaling(HeightRatio);

    switch(layer.Type)
    {
    case TextLayer_Background:
        return generateRectangle(ren, verOut, mtx, ent->EntryData.BackgroundData.Coord, 
            ent->mColor, ent->EntryData.BackgroundData.BorderColor, 
            meshGenFlags);

    case TextLayer_Selection:
        return generateSelection(ren, verOut, layer, mtx, meshGenFlags);

    case TextLayer_Shadow:
    case TextLayer_ShadowText:
    case TextLayer_RasterText:
        return generateRasterMesh(verOut, layer);

    case TextLayer_PackedText:
    case TextLayer_PackedDFAText:
        return generatePackedMesh(verOut, layer);

    case TextLayer_Images:
        return generateImageMesh(verOut, layer);

    case TextLayer_Shapes:
    case TextLayer_Shapes_Masked:
        SF_ASSERT(0); // Vector glyphs are processed in a separate mesh provider.
        break;

    case TextLayer_Underline:
    case TextLayer_Underline_Masked:
        return generateUnderlines(ren, verOut, layer, mtx, meshGenFlags);

    case TextLayer_Cursor:
        return generateRectangle(ren, verOut, mtx, ent->EntryData.SelectionData.Coord, 
            ent->mColor, 0, meshGenFlags);

    case TextLayer_Mask: // Mask rectangle shape.
        return generateMask(verOut, layer);
    }

    return false;
}


//------------------------------------------------------------------------
void TextMeshProvider::GetFillData(FillData* data, unsigned layer,
                                   unsigned fillIndex, unsigned meshGenFlags)
{
    SF_UNUSED(fillIndex);
    if (meshGenFlags & Mesh_Mask)

    {
        *data = FillData(Fill_Mask);
        return;
    }

    switch(Layers[layer].Type)
    {
    case TextLayer_Background:
    case TextLayer_Selection:
    case TextLayer_Shapes:
    case TextLayer_Underline:
    case TextLayer_Cursor:
    case TextLayer_Shapes_Masked:
    case TextLayer_Underline_Masked:
        {
            *data = FillData(Fill_VColor);
            data->PrimFill = PrimFill_VColor;
            data->pVFormat = &VertexXY16iCF32::Format;
            break;
        }

    case TextLayer_Shadow:
    case TextLayer_ShadowText:
    case TextLayer_RasterText:
        {
            Image* img = pCache->GetImage(Entries[Layers[layer].Start].TextureId);
            *data = FillData(img, ImageFillMode(Wrap_Clamp, Sample_Linear));
            data->PrimFill = PrimFill_UVTextureAlpha_VColor;
            data->pVFormat = &RasterGlyphVertex::Format;
            break;
        }

    case TextLayer_PackedText:
        {
            Image* img = Entries[Layers[layer].Start].EntryData.PackedData.pGlyph->pImage;
            *data = FillData(img, ImageFillMode(Wrap_Clamp, Sample_Linear));
            if (img->GetFormat() == Image_A8)
            {
                data->PrimFill = PrimFill_UVTextureAlpha_VColor;
                data->pVFormat = &RasterGlyphVertex::Format;
            }
            else
            {
                data->PrimFill = PrimFill_UVTexture;
                data->pVFormat = &ImageGlyphVertex::Format;
            }
            break;
        }

    case TextLayer_PackedDFAText:
        {
            Image* img = Entries[Layers[layer].Start].EntryData.PackedData.pGlyph->pImage;
            *data = FillData(img, ImageFillMode(Wrap_Clamp, Sample_Linear));
            data->PrimFill = PrimFill_UVTextureDFAlpha_VColor;
            data->pVFormat = &RasterGlyphVertex::Format;
            break;
        }

    case TextLayer_Images:
        {
            unsigned idx = Layers[layer].Start;
            Image* img = Entries[idx].EntryData.ImageData.pImage;
            *data = FillData(img, ImageFillMode(Wrap_Clamp, Sample_Linear));
            data->PrimFill = PrimFill_UVTexture;
            data->pVFormat = &ImageGlyphVertex::Format;
            break;
        }

    case TextLayer_Mask:
        {
            *data = FillData(Fill_Mask);
            break;
        }
    }
}






//------------------------------------------------------------------------
void TextMeshProvider::addVectorGlyph(TmpTextStorage& storage, UInt32 color, FontCacheHandle* font, 
                                      UInt16 glyphIndex, UInt16 flags, 
                                      float fontSize, float x, float y)
{
    TmpTextMeshEntry e;
    e.LayerType = TextLayer_Shapes;
    e.TextureId = 0;
    e.EntryIdx  = (unsigned)storage.Entries.GetSize();
    e.mColor    = color;
    e.pFill     = pCache->GetFill(TextLayer_Shapes, 0); 
    e.pFill->AddRef();
    font->pFont->AddRef();
    e.EntryData.VectorData.pFont      = font->pFont;
    e.EntryData.VectorData.GlyphIndex = glyphIndex;
    e.EntryData.VectorData.Flags      = flags;
    e.EntryData.VectorData.FontSize   = fontSize;
    e.EntryData.VectorData.x          = x;
    e.EntryData.VectorData.y          = y;
    storage.Entries.PushBack(e);
}

//------------------------------------------------------------------------
void TextMeshProvider::addBackground(TmpTextStorage& storage, UInt32 color, UInt32 borderColor, const RectF& rect)
{
    TmpTextMeshEntry e;
    e.LayerType = TextLayer_Background;
    e.TextureId = 0;
    e.EntryIdx  = (unsigned)storage.Entries.GetSize();
    e.mColor    = color;
    e.pFill     = pCache->GetFill(TextLayer_Background, 0);
    e.pFill->AddRef();
    e.EntryData.BackgroundData.Coord[0] = rect.x1;
    e.EntryData.BackgroundData.Coord[1] = rect.y1;
    e.EntryData.BackgroundData.Coord[2] = rect.x2;
    e.EntryData.BackgroundData.Coord[3] = rect.y2;
    e.EntryData.BackgroundData.BorderColor = borderColor;
    storage.Entries.PushBack(e);
}

//------------------------------------------------------------------------
void TextMeshProvider::addSelection(TmpTextStorage& storage, UInt32 color, const RectF& rect)
{
    TmpTextMeshEntry e;
    e.LayerType = TextLayer_Selection;
    e.TextureId = 0;
    e.EntryIdx  = (unsigned)storage.Entries.GetSize();
    e.mColor    = color;
    e.pFill     = pCache->GetFill(TextLayer_Selection, 0);
    e.pFill->AddRef();
    e.EntryData.SelectionData.Coord[0] = rect.x1;
    e.EntryData.SelectionData.Coord[1] = rect.y1;
    e.EntryData.SelectionData.Coord[2] = rect.x2;
    e.EntryData.SelectionData.Coord[3] = rect.y2;
    storage.Entries.PushBack(e);
}

//------------------------------------------------------------------------
void TextMeshProvider::addCursor(TmpTextStorage& storage, UInt32 color, const RectF& rect)
{
    TmpTextMeshEntry e;
    e.LayerType = TextLayer_Cursor;
    e.TextureId = 0;
    e.EntryIdx  = (unsigned)storage.Entries.GetSize();
    e.mColor    = color;
    e.pFill     = pCache->GetFill(TextLayer_Cursor, 0);
    e.pFill->AddRef();
    e.EntryData.SelectionData.Coord[0] = rect.x1;
    e.EntryData.SelectionData.Coord[1] = rect.y1;
    e.EntryData.SelectionData.Coord[2] = rect.x2;
    e.EntryData.SelectionData.Coord[3] = rect.y2;
    storage.Entries.PushBack(e);
}

//------------------------------------------------------------------------
void TextMeshProvider::addUnderline(TmpTextStorage& storage, UInt32 color, TextUnderlineStyle style, float x, float y, float len)
{
    TmpTextMeshEntry e;
    e.LayerType = TextLayer_Underline;
    e.TextureId = 0;
    e.EntryIdx  = (unsigned)storage.Entries.GetSize();
    e.mColor    = color;
    e.pFill     = pCache->GetFill(TextLayer_Underline, 0);
    e.pFill->AddRef();
    e.EntryData.UnderlineData.Style = style;
    e.EntryData.UnderlineData.x     = x;
    e.EntryData.UnderlineData.y     = y;
    e.EntryData.UnderlineData.Len   = len;
    storage.Entries.PushBack(e);
}

//------------------------------------------------------------------------
void TextMeshProvider::addMask(TmpTextStorage& storage)
{
    TmpTextMeshEntry e;
    e.LayerType = TextLayer_Mask;
    e.TextureId = 0;
    e.EntryIdx  = (unsigned)storage.Entries.GetSize();
    e.mColor    = 0;
    e.pFill     = pCache->GetFill(TextLayer_Mask, 0);
    e.pFill->AddRef();
    e.EntryData.MaskData.Coord[0] = ClipBox.x1;
    e.EntryData.MaskData.Coord[1] = ClipBox.y1;
    e.EntryData.MaskData.Coord[2] = ClipBox.x2;
    e.EntryData.MaskData.Coord[3] = ClipBox.y2;
    storage.Entries.PushBack(e);
}

//------------------------------------------------------------------------
void TextMeshProvider::addLayer(TmpTextStorage& storage, TextLayerType type, unsigned start, unsigned count)
{
    enum { MaxCapacity = 16382 };
    TmpTextMeshLayer layer;

    do
    {
        unsigned c2 = (count < MaxCapacity) ? count : MaxCapacity;
        layer.Type = type;
        layer.Start = start;
        layer.Count = c2;
        layer.pFill = storage.Entries[start].pFill;
        storage.Layers.PushBack(layer);
        count -= c2;
        start += c2;
    } while(count);
}

//------------------------------------------------------------------------
void TextMeshProvider::addLayer(TmpTextStorage& storage, UPInt start, UPInt end)
{
   // Merge shadow and text layers if possible
    if (storage.Entries[start].LayerType == TextLayer_RasterText && Layers.GetSize())
    {
        TextMeshLayer& prevLayer = Layers.Back();
        if ((prevLayer.Type == TextLayer_Shadow || prevLayer.Type == TextLayer_ShadowText) &&
            storage.Entries[start].pFill == storage.Entries[prevLayer.Start].pFill)
        {
            prevLayer.Type  = TextLayer_ShadowText;
            prevLayer.Count += unsigned(end - start);
            return;
        }
    }

    TextMeshLayer layer;

    // Use separate layers for vector Shapes 
    if (storage.Entries[start].LayerType == TextLayer_Shapes || 
        storage.Entries[start].LayerType == TextLayer_Shapes_Masked)
    {
        for(UPInt i = start; i < end; ++i)
            addLayer(storage, (TextLayerType)storage.Entries[start].LayerType, (unsigned)i, 1);
        return;
    }

    // Use separate layers for images 
    if (storage.Entries[start].LayerType == TextLayer_Images)
    {
        for(UPInt i = start; i < end; ++i)
            addLayer(storage, TextLayer_Images, (unsigned)i, 1);
        return;
    }

    addLayer(storage, (TextLayerType)storage.Entries[start].LayerType, (unsigned)start, unsigned(end - start));
}


//------------------------------------------------------------------------
bool TextMeshProvider::CmpEntries::operator() (const TmpTextMeshEntry& a, const TmpTextMeshEntry& b) const
{
    if (a.LayerType != b.LayerType) 
        return a.LayerType < b.LayerType;

    if (a.pFill != b.pFill)
        return a.pFill < b.pFill;

    return a.EntryIdx < b.EntryIdx;
}

//------------------------------------------------------------------------
void TextMeshProvider::sortEntries(TmpTextStorage& storage)
{
    storage.Layers.Clear();

    CmpEntries cmp(pCache);
    Alg::QuickSort(storage.Entries, cmp);

    UPInt start = 0;
    UPInt end = 0;
    for(; end < storage.Entries.GetSize(); ++end)
    {
        if (storage.Entries[start].LayerType != storage.Entries[end].LayerType || 
            storage.Entries[start].pFill != storage.Entries[end].pFill)
        {
            if (end > start)
            {
                addLayer(storage, start, end);
                start = end;
            }
        }
    }
    if (end > start)
    {
        addLayer(storage, start, end);
    }

    Entries.Resize(storage.Entries.GetSize());
    Layers.Resize(storage.Layers.GetSize());
    UPInt i;
    for (i = 0; i < storage.Entries.GetSize(); ++i)
    {
        const TmpTextMeshEntry& e1 = storage.Entries[i];
        TextMeshEntry& e2 = Entries[i];
        e2.LayerType = e1.LayerType;
        e2.TextureId = e1.TextureId;
        e2.mColor    = e1.mColor;
        e2.pFill     = *e1.pFill;
        e2.EntryData = e1.EntryData;
    }

    for (i = 0; i < storage.Layers.GetSize(); ++i)
    {
        const TmpTextMeshLayer& l1 = storage.Layers[i];
        TextMeshLayer& l2 = Layers[i];
        l2.Type  = l1.Type;
        l2.Start = l1.Start;
        l2.Count = l1.Count;
        l2.pMesh = 0;
        l2.M     = HMatrix();
        l2.pFill = l1.pFill;
        l2.SizeScale = 1;
    }
}


//------------------------------------------------------------------------
void TextMeshProvider::addImage(TmpTextStorage& storage, const GlyphRunData& data, 
                                Image* img, float scaleX, float scaleY, float baseLine, bool snap)
{
    float data_NewLineX = data.NewLineX;
    float data_NewLineY = data.NewLineY - baseLine * scaleY;
    if (snap)
        data_NewLineX = snapX(data);

    TmpTextMeshEntry e;
    e.LayerType = (UInt16)TextLayer_Images;
    e.TextureId = 0;
    e.EntryIdx  = (unsigned)storage.Entries.GetSize();

    PrimitiveFillData fillData(PrimFill_UVTexture, &ImageGlyphVertex::Format, 
                               img->GetTexture(pCache->GetTextureManager()), 
                               ImageFillMode(Wrap_Clamp, Sample_Linear));

    e.pFill = pCache->GetPrimitiveFillManager()->CreateFill(fillData);

    e.EntryData.ImageData.Coord[0] = data_NewLineX;
    e.EntryData.ImageData.Coord[1] = data_NewLineY;
    e.EntryData.ImageData.Coord[2] = data_NewLineX + (float)img->GetSize().Width * scaleX;
    e.EntryData.ImageData.Coord[3] = data_NewLineY + (float)img->GetSize().Height * scaleY;
    e.EntryData.ImageData.pImage   = img;
    storage.Entries.PushBack(e);
}


//------------------------------------------------------------------------
float TextMeshProvider::calcHeightRatio(const HMatrix& m, const Matrix4F& m4, const Viewport& vp)
{
    Matrix2F matrix = m.GetMatrix2D();
    float hr;
    if (m.Has3D())
    {
        RectF bounds(1,1);
        SF_SIMD_ALIGN(float coord[8]);
        m4.TransformHomogeneousAndScaleCorners(bounds, (float)vp.Width, (float)vp.Height, coord);
        matrix.SetRectToParl(0, 0, 1, 1, coord);

        hr = matrix.GetMaxScale();
        return (hr < 1e-5f) ? 1e-5f : hr;
    }

    float x01=0, y01=1, x10=1, y10=0;
    matrix.Transform2x2(&x01, &y01);
    matrix.Transform2x2(&x10, &y10);
    hr = fabsf(Math2D::LinePointDistance(0, 0, x10, y10, x01, y01));
    return (hr < 1e-8f) ? 1e-8f : hr;
}

//------------------------------------------------------------------------
bool TextMeshProvider::NeedsUpdate(const HMatrix& m, const Matrix4F& m4, const Viewport& vp, 
                                   const TextFieldParam& param) const
{
    // TO DO: make a decision depending on the flags such ad OptRead 
    float heightRatio = calcHeightRatio(m, m4, vp);

    float k1 = 0.85f; // TO DO: Move to Tolerance Param
    if (param.TextParam.IsOptRead())
        k1 = 0.99f;

    return heightRatio < HeightRatio * k1 || heightRatio > HeightRatio / k1;
}


//------------------------------------------------------------------------
void TextMeshProvider::addRasterGlyph(TmpTextStorage& storage, TextLayerType type, GlyphRunData& data, 
                                      UInt32 color, GlyphNode* node, float screenSize, bool snap, float stretch)
{
    float data_NewLineX = data.NewLineX;
    if (snap && 
        (data.pFont->IsRasterOnly() || 
        (node->Param.IsOptRead() && node->Param.GetStretch() <= 1)))
    {
        data_NewLineX = snapX(data);
    }

    UInt16 textureId = pCache->GetTextureId(node);
    const GlyphRect& r = node->mRect;

    if (r.w > 1 && r.h > 1)
    {
        RectF rect;
        rect.x1 = (float(node->Origin.x) / 16.0f) + 1.0f;
        rect.y1 = (float(node->Origin.y) / 16.0f) + 1.0f;
        rect.x2 = rect.x1 + r.w - 2.0f;
        rect.y2 = rect.y1 + r.h - 2.0f;

        float size = node->Param.GetFontSize();
        if (size < 1e-5f)
            size = 1e-5f;
        float k = (node->Scale * screenSize / size) / data.HeightRatio; 

//printf("%d", stretch); // DBG
        rect.x1 *= k / stretch;
        rect.y1 *= k;
        rect.x2 *= k / stretch;
        rect.y2 *= k;

        float offsetX = 0;
        float offsetY = 0;
        if (type == TextLayer_Shadow)
        {
            offsetX = data.Param.ShadowOffsetX;
            offsetY = data.Param.ShadowOffsetY;
        }

        rect.x1 += data_NewLineX + offsetX;
        rect.y1 += data.NewLineY + offsetY;
        rect.x2 += data_NewLineX + offsetX;
        rect.y2 += data.NewLineY + offsetY;

        TmpTextMeshEntry e;
        e.LayerType = (UInt16)type;
        e.TextureId = textureId;
        e.EntryIdx  = (unsigned)storage.Entries.GetSize();
        e.mColor    = color;
        e.pFill     = pCache->GetFill(type, textureId);
        e.pFill->AddRef();
        e.EntryData.RasterData.Coord[0] = rect.x1;
        e.EntryData.RasterData.Coord[1] = rect.y1;
        e.EntryData.RasterData.Coord[2] = rect.x2;
        e.EntryData.RasterData.Coord[3] = rect.y2;
        e.EntryData.RasterData.pGlyph = node;
        storage.Entries.PushBack(e);
    }
}


//------------------------------------------------------------------------
void TextMeshProvider::addTextureGlyph(TmpTextStorage& storage, const TextureGlyph* tgl, 
                                       GlyphRunData& data, UInt32 color)
{
    float data_NewLineX = data.NewLineX;
    //if (snap && data.Param.TextParam.IsOptRead())
    //{
    //    data_NewLineX = snapX(data);
    //}

    ImageSize s = tgl->pImage->GetSize();

    float scaleX = data.FontSize / data.TexHeight * s.Width;
    float scaleY = data.FontSize / data.TexHeight * s.Height;

    float x1 = (tgl->UvBounds.x1 - tgl->UvOrigin.x) * scaleX;
    float y1 = (tgl->UvBounds.y1 - tgl->UvOrigin.y) * scaleY;
    float x2 = (tgl->UvBounds.x2 - tgl->UvOrigin.x) * scaleX;
    float y2 = (tgl->UvBounds.y2 - tgl->UvOrigin.y) * scaleY;

    x1 += data_NewLineX;
    y1 += data.NewLineY;
    x2 += data_NewLineX;
    y2 += data.NewLineY;

    TmpTextMeshEntry e;
    e.LayerType = (UInt16)TextLayer_PackedText;
    e.TextureId = 0;
    e.EntryIdx  = (unsigned)storage.Entries.GetSize();
    e.mColor    = color;

    if (data.pFont->GetFontFlags() & Font::FF_DistanceFieldAlpha)
    {
        PrimitiveFillData fillData(PrimFill_UVTextureDFAlpha_VColor, &RasterGlyphVertex::Format, 
                                   tgl->pImage->GetTexture(pCache->GetTextureManager()), 
                                   ImageFillMode(Wrap_Clamp, Sample_Linear));
        e.pFill = pCache->GetPrimitiveFillManager()->CreateFill(fillData);
        e.LayerType = (UInt16)TextLayer_PackedDFAText;
        Flags |= BF_DistField;
    }
    else if (tgl->pImage->GetFormat() == Image_A8)
    {
        PrimitiveFillData fillData(PrimFill_UVTextureAlpha_VColor, &RasterGlyphVertex::Format, 
                                   tgl->pImage->GetTexture(pCache->GetTextureManager()), 
                                   ImageFillMode(Wrap_Clamp, Sample_Linear));
        e.pFill = pCache->GetPrimitiveFillManager()->CreateFill(fillData);
    }
    else
    {
        PrimitiveFillData fillData(PrimFill_UVTexture, &ImageGlyphVertex::Format, 
                                   tgl->pImage->GetTexture(pCache->GetTextureManager()), 
                                   ImageFillMode(Wrap_Clamp, Sample_Linear));
        e.pFill = pCache->GetPrimitiveFillManager()->CreateFill(fillData);
    }

    e.EntryData.PackedData.Coord[0] = x1;
    e.EntryData.PackedData.Coord[1] = y1;
    e.EntryData.PackedData.Coord[2] = x2;
    e.EntryData.PackedData.Coord[3] = y2;
    e.EntryData.PackedData.pGlyph = tgl;
    storage.Entries.PushBack(e);
}


//------------------------------------------------------------------------
bool TextMeshProvider::addGlyph(TmpTextStorage& storage, GlyphRunData& data, unsigned glyphIndex, 
                                bool fauxBold, bool fauxItalic, bool snap, unsigned meshGenFlags)
{
    bool needVector = false;
    bool drawShadow = true;
    float screenSize = data.FontSize * data.HeightRatio;
    GlyphParam gp;
    GlyphNode* node;

    const TextureGlyph* tgl = data.pFont->GetTextureGlyph(glyphIndex);
    if (tgl)
    {
        if (data.pFont->GlyphShapesStripped() || screenSize <= data.TexHeight * pCache->GetMaxRasterScale())
        {
            if (tgl->pImage)
                addTextureGlyph(storage, tgl, data, data.mColor);
            return true;
        }
        if (!data.pFont->GlyphShapesStripped())
        {
            needVector = true;
            drawShadow = false;
        }
    }

    if (meshGenFlags & Mesh_Mask)
        needVector = true;

    Ptr<VectorGlyphShape> sh = 
        *pCache->CreateGlyphShape(data, glyphIndex, screenSize, 
                                  data.Param.TextParam.IsFauxBold() | fauxBold,
                                  data.Param.TextParam.IsFauxItalic() | fauxItalic, 
                                  data.Param.TextParam.GetOutline());

    gp = data.Param.TextParam;
    gp.pFont = data.pFontHandle;
    gp.GlyphIndex = (UInt16)glyphIndex;
    gp.SetFauxBold(data.Param.TextParam.IsFauxBold() | fauxBold);
    gp.SetFauxItalic(data.Param.TextParam.IsFauxItalic() | fauxItalic);
    gp.SetFontSize(pCache->GetCachedFontSize(gp, screenSize, data.pRaster != 0));
    gp.SetUseRaster(data.RasterSize > 0);
    gp.SetStretch(false);

    if ( gp.IsOptRead() && 
        !gp.IsBitmapFont() &&
         data.VectorSize == 0 &&    // Hinted Vector Size
         data.RasterSize == 0 &&    // Hinted Raster Size
        !gp.pFont->pFont->IsRasterOnly() &&
         gp.GetBlurX() == 0 && gp.GetBlurY() == 0)
    {
        // Small glyphs optimized for readability are stretched 
        float w = gp.pFont->pFont->GetGlyphWidth(gp.GlyphIndex) * screenSize / gp.pFont->pFont->GetNominalGlyphHeight();
        if (3*w < pCache->GetMaxGlyphHeight())
            gp.SetStretch(true);
    }

    bool autoFit =
       pCache->GetParams().UseAutoFit && snap && gp.GetOutline() == 0 && 
       gp.IsOptRead() && gp.IsAutoFit() && screenSize > 6 && 
       (gp.pFont->pFont->HasLayout() || gp.pFont->pFont->IsDeviceFont());

    gp.SetAutoFit(autoFit);

    if (!data.Param.ShadowParam.IsKnockOut() && !needVector)
    {
        float stretch = gp.GetStretch();
        node = pCache->FindGlyph(this, gp);
        if (node)
        {
            addRasterGlyph(storage, TextLayer_RasterText, data, data.mColor, node, screenSize, snap, stretch);
        }
        else
        {
            if (gp.BlurX || gp.BlurY)
                node = pCache->RasterizeShadow(data, this, gp, screenSize, data.pRaster);
            else
                node = pCache->RasterizeGlyph(data, this, gp);

            if (node)
            {
                addRasterGlyph(storage, TextLayer_RasterText, data, data.mColor, node, screenSize, snap, stretch);
            }
            else
            {
                GlyphCache::RasResult res = pCache->GetRasResult();

                if (res == GlyphCache::Res_ShapeNotFound)
                {
                    float w = data.FontSize / 4;
                    float h = data.FontSize / 2;
                    RectF bounds(data.NewLineX+w/2, data.NewLineY-h-h/4, data.NewLineX+w+w/2, data.NewLineY-h/4);
                    addSelection(storage, data.mColor, bounds);
                    return true;
                }

                if (res == GlyphCache::Res_ShapeIsEmpty)
                    return true;

                if (res == GlyphCache::Res_CacheFull)
                {
                    if (pCache->GetParams().UseVectorOnFullCache)
                        needVector = true;
                    else
                        return false;
                }

                if (res == GlyphCache::Res_ShapeIsTooBig || res == GlyphCache::Res_NoRasterCache)
                    needVector = true;
            }
        }
    }

    if (needVector && data.pShape && !data.pShape->IsEmpty())
    {
        addVectorGlyph(storage, data.mColor, data.pFontHandle, (UInt16)glyphIndex, gp.Flags, 
                       data.FontSize, data.NewLineX, data.NewLineY);
        Flags |= BF_HasVectorGlyphs;
    }

    if (data.Param.ShadowColor && drawShadow)
    {
        UInt32 color = data.Param.ShadowColor;
        gp = data.Param.ShadowParam;

        gp.SetOptRead(false);
        gp.SetAutoFit(false);

        gp.pFont = data.pFontHandle;
        gp.SetFauxBold(data.Param.TextParam.IsFauxBold() | fauxBold);
        gp.SetFauxItalic(data.Param.TextParam.IsFauxItalic() | fauxItalic);
        gp.SetFontSize(pCache->GetCachedShadowSize(screenSize, data.pRaster));
        gp.GlyphIndex = (UInt16)glyphIndex;

        node = pCache->FindGlyph(this, gp);
        if (node)
        {
            // add raster shadow
            addRasterGlyph(storage, TextLayer_Shadow, data, color, node, screenSize, snap, 1);
        }
        else
        {
            node = pCache->RasterizeShadow(data, this, gp, screenSize, data.pRaster);
            if (node)
            {
                // add raster shadow
                addRasterGlyph(storage, TextLayer_Shadow, data, color, node, screenSize, snap, 1);
            }
            else
            {
                if (pCache->GetRasResult() == GlyphCache::Res_CacheFull)
                    return false;
            }
        }
    }

    return true;
}


//------------------------------------------------------------------------
float TextMeshProvider::snapX(const GlyphRunData& data) const
{
    float x = data.NewLineX;
    float y = data.NewLineY;
    data.DirMtx.Transform(&x, &y);
    x = floorf(x + 0.5f);
    data.InvMtx.Transform(&x, &y);
    return x;
}

//------------------------------------------------------------------------
float TextMeshProvider::snapY(const GlyphRunData& data) const
{
    float x = data.NewLineX;
    float y = data.NewLineY;
    data.DirMtx.Transform(&x, &y);
    y = floorf(y + 0.5f);
    data.InvMtx.Transform(&x, &y);
    return y;
}

//------------------------------------------------------------------------
// static
unsigned TextMeshProvider::CalcVectorParams(TextMeshLayer& layer, const TextMeshEntry& ent, 
                                            const Matrix2F& scalingMtx, 
                                            float sizeScale, const HMatrix& m, 
                                            Renderer2DImpl* ren, unsigned meshGenFlags, float* keyData)
{
    const GlyphVectorEnt& vec = ent.EntryData.VectorData;

    if (m.Has3D())
    {
        Matrix2F m2d = Matrix2F::Scaling(sizeScale);
        m2d.AppendTranslation(vec.x, vec.y);
        Matrix3F m3d = m.GetMatrix3D();
        m3d.Prepend(m2d);
        if (layer.M.IsNull())
            layer.M = ren->GetMatrixPool().CreateMatrix(m3d);
        else 
            layer.M.SetMatrix3D(m3d);
    }
    else
    {
        Matrix2F m2d = m.GetMatrix2D();
        m2d.PrependTranslation(vec.x, vec.y);
        m2d.PrependScaling(sizeScale);
        if (layer.M.IsNull())
            layer.M = ren->GetMatrixPool().CreateMatrix(m2d);
        else 
            layer.M.SetMatrix2D(m2d);
    }

    Cxform colorCx(Color(ent.mColor));
    colorCx.Append(m.GetCxform());
    layer.M.SetCxform(colorCx);

    unsigned flags = MeshKey::KF_Fill;
    if (meshGenFlags & Mesh_EdgeAA) flags |= MeshKey::KF_EdgeAA;
    if (meshGenFlags & Mesh_Mask)   flags |= MeshKey::KF_Mask;    
    bool validKey = MeshKey::CalcMatrixKey(scalingMtx, keyData, 0);
    keyData[MeshKey::GetKeySize(flags) - 1] = 0; // MorhpRatio
    if (!validKey) 
        flags |= MeshKey::KF_Degenerate;
    return flags;
}

//------------------------------------------------------------------------
void TextMeshProvider::createVectorGlyph(unsigned layerIdx, Renderer2DImpl* ren,
                                         const HMatrix& m, unsigned meshGenFlags)
{
    TextMeshLayer& layer = Layers[layerIdx];
    TextMeshEntry& ent = Entries[layer.Start];

    GlyphRunData data;
    data.pFont      = ent.EntryData.VectorData.pFont;
    data.pFontHandle= pCache->RegisterFont(data.pFont);
    data.FontSize   = ent.EntryData.VectorData.FontSize;
    data.VectorSize = 0;
    data.RasterSize = 0;
    data.NomWidth   = 0;
    data.NomHeight  = 0;
    data.TexHeight  = 0;
    data.mColor     = ent.mColor;
    data.NewLineX   = ent.EntryData.VectorData.x;
    data.NewLineY   = ent.EntryData.VectorData.y;
    data.GlyphBounds= RectF(0,0,0,0);
    data.HeightRatio= HeightRatio;
    data.HintedNomHeight = 0;

    GlyphParam gp;
    gp.Flags = ent.EntryData.VectorData.Flags;

    Ptr<VectorGlyphShape> shape = 
        *pCache->CreateGlyphShape(data, ent.EntryData.VectorData.GlyphIndex, (float)data.VectorSize, 
                                  gp.IsFauxBold(), gp.IsFauxItalic(), gp.GetOutline(), true);

    SF_ASSERT(shape);
    SF_ASSERT(data.pShape);

    data.HintedNomHeight = shape->pShape->GetHintedSize();

    float sizeScale = data.FontSize / data.NomHeight;
    SF_SIMD_ALIGN( float keyData[MeshKey::MaxKeySize] );

    Matrix2F scaling = Matrix2F::Scaling(HeightRatio * sizeScale);
    unsigned keyFlags = CalcVectorParams(layer, ent, scaling, sizeScale, m, ren, meshGenFlags, keyData);

    const ToleranceParams& cfg = ren->GetToleranceParams();
    layer.pMeshKey = *ren->GetMeshKeyManager()->CreateMatchingKey(shape, 0, keyFlags, keyData, cfg);
    layer.pShape = shape;
    layer.SizeScale = sizeScale;

    if (!layer.pMeshKey->pMesh)
    {
        layer.pMeshKey->pMesh = *SF_HEAP_AUTO_NEW(this)
            Mesh(ren, layer.pMeshKey->GetKeySet(), scaling, 0, 0, meshGenFlags);
    }
    layer.pMesh = (Mesh*)layer.pMeshKey->pMesh.GetPtr();
}

//------------------------------------------------------------------------
bool TextMeshProvider::CreateMeshData(const TextLayout* layout, Renderer2DImpl* ren, 
                                      const HMatrix& m, const Matrix4F& m4, 
                                      const Viewport& vp, unsigned meshGenFlags)
{
    SF_ASSERT(pRenderer == 0 || pRenderer == ren);
    pRenderer = ren;

    TmpTextStorage storage(Memory::GetGlobalHeap());

    Flags &= ~(BF_Created | BF_HasVectorGlyphs | BF_HasUnderlines);
    Matrix2F viewMatrix = m.GetMatrix2D();
    bool snap = !(m.Has3D() || viewMatrix.IsFreeRotation());
    SF_ASSERT(Layers.GetSize() == 0);
    SF_ASSERT(Entries.GetSize() == 0);
    GlyphRunData data;
    data.Param      = layout->GetParam();
    data.Bounds     = layout->GetBounds();
    data.pFont      = 0;
    data.pFontHandle= 0;
    data.FontSize   = 0;
    data.VectorSize = 0;
    data.RasterSize = 0;
    data.NomWidth   = 0;
    data.NomHeight  = 0;
    data.TexHeight  = 0;
    data.mColor     = 0;
    data.NewLineX   = 0;
    data.NewLineY   = 0;
    data.GlyphBounds= RectF(0,0,0,0);
    data.DirMtx     = m.GetMatrix2D();
    data.InvMtx     = data.DirMtx.GetInverse();
    data.HeightRatio= calcHeightRatio(m, m4, vp);
    data.HintedNomHeight = 0;

    HeightRatio = data.HeightRatio;

    if (m.Has3D()) // TO DO: Check config
    {
        data.Param.TextParam.SetOptRead(false);
        data.Param.TextParam.SetAutoFit(false);
        data.Param.ShadowParam.SetOptRead(false);
        data.Param.ShadowParam.SetAutoFit(false);
    }

    Bounds = layout->GetBounds();
    Flags &= ~BF_Clip;
    ClipBox = layout->GetClipBox();
    if (!ClipBox.IsEmpty())
        Flags |= BF_Clip;

    Font* font = 0;

    Flags |= BF_Creating;

    TextLayout::Record rec;
    UPInt pos = 0;
    bool cacheIsOK = true;
    while(cacheIsOK && (pos = layout->ReadNext(pos, &rec)) != 0)
    {
        switch(layout->GetRecordType(rec))
        {
        case TextLayout::Record_Char:
            SF_ASSERT(data.pFont != 0);
            if ((rec.mChar.Flags & TextLayout::Flag_Invisible) == 0)
            {
                cacheIsOK = addGlyph(storage, data, rec.mChar.GlyphIndex, 
                    (rec.mChar.Flags & TextLayout::Flag_FauxBold) != 0,
                    (rec.mChar.Flags & TextLayout::Flag_FauxItalic) != 0,
                    snap, meshGenFlags);
            }
            else
            {
                cacheIsOK = true;
            }
            data.NewLineX += rec.mChar.Advance;
            break;

        case TextLayout::Record_Color:
            data.mColor = rec.mColor.mColor;
            break;

        case TextLayout::Record_Background:
            addBackground(storage, rec.mBackground.BackgroundColor, rec.mBackground.BorderColor, data.Bounds);
            break;

        case TextLayout::Record_NewLine:
            data.NewLineX = rec.mLine.x;
            data.NewLineY = rec.mLine.y;
            if (snap && 
                ((data.pFont && data.pFont->IsRasterOnly()) || 
                 data.Param.TextParam.IsOptRead()))
            {
                data.NewLineY = snapY(data);
            }
            break;

        case TextLayout::Record_Font:
            font = rec.mFont.pFont;
            data.pFont = font;
            data.pFontHandle = pCache->RegisterFont(font);
            data.FontSize    = rec.mFont.mSize;
            data.TexHeight   = font->GetTextureGlyphHeight();
            // Snapping to Y is necessary when changing font has only raster glyphs
            // in order to have crisp glyphs (mostly used in HUDs)
            if (snap && data.pFont && data.pFont->IsRasterOnly())
            {
                data.NewLineY = snapY(data);
            }
            break;

        case TextLayout::Record_Selection:
            addSelection(storage, rec.mSelection.mColor, 
                RectF(rec.mSelection.x1, rec.mSelection.y1, rec.mSelection.x2, rec.mSelection.y2));
            break;

        case TextLayout::Record_Underline:
            addUnderline(storage, rec.mUnderline.mColor, (TextUnderlineStyle)rec.mUnderline.Style, 
                rec.mUnderline.x, rec.mUnderline.y, rec.mUnderline.Len);
            Flags |= BF_HasUnderlines;
            break;

        case TextLayout::Record_Cursor:
            addCursor(storage, rec.mCursor.mColor, 
                RectF(rec.mCursor.x1, rec.mCursor.y1, rec.mCursor.x2, rec.mCursor.y2));
            break;

        case TextLayout::Record_Image:
            addImage(storage, data, rec.mImage.pImage, 
                rec.mImage.ScaleX, rec.mImage.ScaleY, rec.mImage.BaseLine, snap);
            data.NewLineX += rec.mImage.Advance;
            break;

        case TextLayout::Record_RefCntData:
            break;
        }
    }

    UPInt i;
    if ((Flags & BF_Clip) && (Flags & (BF_HasVectorGlyphs | BF_HasUnderlines)))
    {
        addMask(storage);
        bool first = true;
        ClearBox.Clear();
        for (i = 0; i < storage.Entries.GetSize(); ++i)
        {
            TmpTextMeshEntry& ent = storage.Entries[i];
            RectF bounds(0,0,0,0);
            switch(ent.LayerType)
            {
            case TextLayer_RasterText:
                bounds.x1 = ent.EntryData.RasterData.Coord[0];
                bounds.y1 = ent.EntryData.RasterData.Coord[1];
                bounds.x2 = ent.EntryData.RasterData.Coord[2];
                bounds.y2 = ent.EntryData.RasterData.Coord[3];
                break;

            case TextLayer_PackedText:
                bounds.x1 = ent.EntryData.PackedData.Coord[0];
                bounds.y1 = ent.EntryData.PackedData.Coord[1];
                bounds.x2 = ent.EntryData.PackedData.Coord[2];
                bounds.y2 = ent.EntryData.PackedData.Coord[3];
                break;

            case TextLayer_Images:
                bounds.x1 = ent.EntryData.ImageData.Coord[0];
                bounds.y1 = ent.EntryData.ImageData.Coord[1];
                bounds.x2 = ent.EntryData.ImageData.Coord[2];
                bounds.y2 = ent.EntryData.ImageData.Coord[3];
                break;

            case TextLayer_Shapes:
                {
                    const GlyphVectorEnt& vec = ent.EntryData.VectorData;
                    float sizeScale = vec.FontSize / vec.pFont->GetNominalGlyphHeight();
                    vec.pFont->GetGlyphBounds(vec.GlyphIndex, &bounds);
                    bounds.x1 = bounds.x1 * sizeScale + vec.x;
                    bounds.y1 = bounds.y1 * sizeScale + vec.y;
                    bounds.x2 = bounds.x2 * sizeScale + vec.x;
                    bounds.y2 = bounds.y2 * sizeScale + vec.y;
                    ent.LayerType = TextLayer_Shapes_Masked;
                }
                break;
            }

            if (!bounds.IsEmpty())
            {
                if (first)
                    ClearBox = bounds;
                else
                    ClearBox.Union(bounds);
                first = false;
            }
            if (ent.LayerType == TextLayer_Underline)
                ent.LayerType  = TextLayer_Underline_Masked;
        }
        float pad = 1/HeightRatio;
        ClearBox.x1 -= pad;
        ClearBox.y1 -= pad;
        ClearBox.x2 += pad;
        ClearBox.y2 += pad;
        Flags |= BF_HasMask;
    }

    UnpinSlots();
    Flags &= ~BF_Creating;

    sortEntries(storage);

    for(i = 0; i < Layers.GetSize(); ++i)
    {
        TextMeshLayer& layer = Layers[i];
        if (layer.Type == TextLayer_Shapes || layer.Type == TextLayer_Shapes_Masked)
        {
            createVectorGlyph((unsigned)i, ren, m, meshGenFlags);
            Flags |= BF_HasVectorGlyphs;
        }
        else
        {
            layer.M = m;
            layer.pMesh = *SF_HEAP_AUTO_NEW(this)
                Mesh(ren, this, viewMatrix, 0, (unsigned)i, meshGenFlags);
        }
    }

    Flags |= BF_Created;

    //printf("%f\n", (float)pCache->ComputeUsedArea() / (float)pCache->ComputeTotalArea()); // DBG
    return true;
}



}} // Scaleform::Render

