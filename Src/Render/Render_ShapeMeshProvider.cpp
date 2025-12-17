/**************************************************************************

Filename    :   Render_ShapeMeshProvider.cpp
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_ShapeMeshProvider.h"
#include "Render_Math2D.h"
#include "Render_BoundBox.h"
#include "Render_TessCurves.h"
#include "Renderer2DImpl.h"

#include <stdio.h> // remove

namespace Scaleform { namespace Render {



class MorphInterpolator : public ShapeDataInterface
{
public: 
    //--------------------------------------------------------------------
    MorphInterpolator(ShapeDataInterface* shape, MorphShapeData* morph, float ratio, 
                      const ShapePosInfo& pos2)
        : pShapeData(shape), pMorphData(morph), MorphRatio(ratio), Pos2(pos2), Pos2s(pos2)
    {}

    //--------------------------------------------------------------------
    virtual unsigned GetFillStyleCount() const
    {
        return pShapeData->GetFillStyleCount();
    }

    //--------------------------------------------------------------------
    virtual unsigned GetStrokeStyleCount() const
    {
        return pShapeData->GetStrokeStyleCount();
    }

    //--------------------------------------------------------------------
    virtual void GetFillStyle(unsigned idx, FillStyleType* f1) const
    {
        pShapeData->GetFillStyle(idx, f1);
        if (pMorphData.GetPtr() == 0 || MorphRatio == 0)
            return;

        FillStyleType f2;
        pMorphData->pMorphTo->GetFillStyle(idx, &f2);

        if (f1->pFill.GetPtr() == 0)
        {
            f1->Color = Color::Blend(Color(f1->Color), Color(f2.Color), MorphRatio).Raw;
            return;
        }
    }

    //--------------------------------------------------------------------
    virtual void GetStrokeStyle(unsigned idx, StrokeStyleType* s1) const
    {
        pShapeData->GetStrokeStyle(idx, s1); 
        if (pMorphData.GetPtr() == 0 || MorphRatio == 0)
            return;

        StrokeStyleType s2;
        pMorphData->pMorphTo->GetStrokeStyle(idx, &s2);

        s1->Width = Alg::Lerp(s1->Width, s2.Width, MorphRatio);

        if (s1->pFill.GetPtr() == 0)
        {
            s1->Color = Color::Blend(Color(s1->Color), Color(s2.Color), MorphRatio).Raw;
            return;
        }
    }

    //--------------------------------------------------------------------
    virtual unsigned GetStartingPos() const
    {
        const ShapeDataInterface* shape = pMorphData.GetPtr() ? &pMorphData->ShapeData1 : pShapeData;
        return shape->GetStartingPos();
    }

    //--------------------------------------------------------------------
    virtual ShapePathType ReadPathInfo(ShapePosInfo* pos1, float* coord1, unsigned* styles1) const
    {
        if (pMorphData.GetPtr() == 0)
            return pShapeData->ReadPathInfo(pos1, coord1, styles1);

        unsigned styles2[3];
        float coord2[Edge_MaxCoord];

        ShapePathType p1 = pMorphData->ShapeData1.ReadPathInfo(pos1,  coord1, styles1);
        ShapePathType p2 = pMorphData->ShapeData2.ReadPathInfo(&Pos2, coord2, styles2);
        SF_ASSERT(p1 == p2);
        SF_UNUSED(p2);
        if (p1 != Shape_EndShape)
        {
            SF_ASSERT(styles1[0] == styles2[0] && styles1[1] == styles2[1] && styles1[2] == styles2[2]);
            coord1[0] = Alg::Lerp(coord1[0], coord2[0], MorphRatio);
            coord1[1] = Alg::Lerp(coord1[1], coord2[1], MorphRatio);
        }
        return p1;
    }

    //--------------------------------------------------------------------
    virtual PathEdgeType ReadEdge(ShapePosInfo* pos1, float* coord1) const
    {
        if (pMorphData.GetPtr() == 0)
            return pShapeData->ReadEdge(pos1, coord1);

        float coord2[Edge_MaxCoord];
        PathEdgeType p1 = pMorphData->ShapeData1.ReadEdge(pos1,  coord1);
        PathEdgeType p2 = pMorphData->ShapeData2.ReadEdge(&Pos2, coord2);
        SF_ASSERT(p1 == p2);
        SF_UNUSED(p2);
        if (p1 != Edge_EndPath)
        {
            coord1[0] = Alg::Lerp(coord1[0], coord2[0], MorphRatio);
            coord1[1] = Alg::Lerp(coord1[1], coord2[1], MorphRatio);
            if (p1 == Edge_QuadTo || p1 == Edge_CubicTo)
            {
                coord1[2] = Alg::Lerp(coord1[2], coord2[2], MorphRatio);
                coord1[3] = Alg::Lerp(coord1[3], coord2[3], MorphRatio);
            }
            if (p1 == Edge_CubicTo)
            {
                coord1[4] = Alg::Lerp(coord1[4], coord2[4], MorphRatio);
                coord1[5] = Alg::Lerp(coord1[5], coord2[5], MorphRatio);
            }
        }
        return p1;
    }

    //--------------------------------------------------------------------
    virtual void SkipPathData(ShapePosInfo* pos1) const
    {
        if (pMorphData.GetPtr() == 0)
        {
            pShapeData->SkipPathData(pos1);
            return;
        }
        pMorphData->ShapeData1.SkipPathData(pos1);
        pMorphData->ShapeData2.SkipPathData(&Pos2);
    }

    //--------------------------------------------------------------------
    virtual void Rewind() const 
    {
        Pos2 = Pos2s;
    }
    
private:
    //--------------------------------------------------------------------
    Ptr<ShapeDataInterface>     pShapeData;
    Ptr<MorphShapeData>         pMorphData;
    float                       MorphRatio;
    mutable ShapePosInfo        Pos2;  // Since it's used only as an adaptor, mutable is OK here.
            ShapePosInfo        Pos2s; // Saved position for Rewind
};

//------------------------------------------------------------------------
MorphShapeData::MorphShapeData(ShapeDataInterface* morphTo) : 
    pMorphTo(morphTo),
    ShapeData1(&Container1, 0),
    ShapeData2(&Container2, 0)
{}


//------------------------------------------------------------------------
ShapeMeshProvider::ShapeMeshProvider(ShapeDataInterface* shape, ShapeDataInterface* shapeMorph)
    : pShapeData(shape), pMorphData(0), IdentityBounds(), GradientMorph(false), Strokes(false)
{
    if (shapeMorph)
    {
        pMorphData = *SF_HEAP_AUTO_NEW(this) MorphShapeData(shapeMorph);
        createMorphData();
    }
    acquireShapeData();
}

//------------------------------------------------------------------------
ShapeMeshProvider::~ShapeMeshProvider()
{
    // NOTE: If this mesh provider has a delegate key, this will clear it (inside lock). This is 
    // important, because otherwise it is possible that the render thread will access this object 
    // simultaneously after this destructor has exited, but after this derived class has destroyed
    // its members.
    releaseKeySet();

    SF_AMP_CODE(clearStrokeCount();)
}

//------------------------------------------------------------------------
void ShapeMeshProvider::AttachShape(ShapeDataInterface* shape, ShapeDataInterface* shapeMorph)
{
    SF_ASSERT(pShapeData.GetPtr() == 0 && pMorphData.GetPtr() == 0);
    pShapeData = shape;
    if (shapeMorph)
    {
        pMorphData = *SF_HEAP_AUTO_NEW(this) MorphShapeData(shapeMorph);
        createMorphData();
    }
    acquireShapeData();
}


//------------------------------------------------------------------------
void ShapeMeshProvider::createMorphData()
{
    // Create and normalize resulting shapes. The trick here is as follows.
    // Although, both shapes must have exactly the same number of edges
    // they can have different number of paths! That's a bad news because
    // it complicates morphing a lot. On the picture below there's 
    // an example of the situation:
    // Shape1:           *----------*----------
    // Shape2:           *------*-------*------
    // Resulting Shapes: *------*---*---*------
    // Here '*' indicates new path, '-' is an edge. That is, for correct 
    // morphing the initial and target shapes must be split and the 
    // resulting shapes can have more paths than Shape1 or Shape2.
    //--------------------------------------

    SF_ASSERT(pMorphData && pMorphData->pMorphTo);

    ShapeDataInterface* morph1 = pShapeData;
    ShapeDataInterface* morph2 = pMorphData->pMorphTo;


    // Setup gradient morph
    SF_ASSERT(morph1->GetFillStyleCount()   == morph2->GetFillStyleCount());
    SF_ASSERT(morph1->GetStrokeStyleCount() == morph2->GetStrokeStyleCount());

    unsigned i;
    for(i = 0; i < morph1->GetFillStyleCount(); ++i)
    {
        FillStyleType fs1;
        morph1->GetFillStyle(i+1, &fs1);
        if (fs1.pFill.GetPtr() && fs1.pFill->pGradient.GetPtr())
        {
            FillStyleType fs2;
            morph2->GetFillStyle(i+1, &fs2);
            SF_ASSERT(fs2.pFill.GetPtr() && fs2.pFill->pGradient.GetPtr());
            bool equals = (*fs1.pFill->pGradient) == (*fs2.pFill->pGradient);
            if (!equals)
            {
                fs1.pFill->pGradient->SetMorphTo(fs2.pFill->pGradient);
                GradientMorph = true;
            }
        }
    }

    for(i = 0; i < morph1->GetStrokeStyleCount(); ++i)
    {
        StrokeStyleType ss1;
        morph1->GetStrokeStyle(i+1, &ss1);
        if (ss1.pFill.GetPtr() && ss1.pFill->pGradient.GetPtr())
        {
            StrokeStyleType ss2;
            morph2->GetStrokeStyle(i+1, &ss2);
            SF_ASSERT(ss2.pFill.GetPtr() && ss2.pFill->pGradient.GetPtr());
            bool equals = (*ss1.pFill->pGradient) == (*ss2.pFill->pGradient);
            if (!equals)
            {
                ss1.pFill->pGradient->SetMorphTo(ss2.pFill->pGradient);
                GradientMorph = true;
            }
        }
    }

    unsigned startPos1 = morph1->GetStartingPos();
    unsigned startPos2 = morph2->GetStartingPos();
    ShapePosInfo posInfo1(startPos1);
    ShapePosInfo posInfo2(startPos2);

    ShapePathType pathType1;
    ShapePathType pathType2;
    PathEdgeType  edgeType1;
    PathEdgeType  edgeType2;
    float coord1[Edge_MaxCoord];
    float coord2[Edge_MaxCoord];
    float start1[2];
    float start2[2];
    unsigned styles1[3];
    unsigned styles2[3];

    pMorphData->ShapeData1.Clear();
    pMorphData->ShapeData2.Clear();

    bool done = false;
    bool first = true;
    while(!done)
    {
        pathType1 = morph1->ReadPathInfo(&posInfo1, coord1, styles1);
        pathType2 = morph2->ReadPathInfo(&posInfo2, coord2, styles2);

        if (pathType1 == Shape_EndShape || pathType2 == Shape_EndShape)
        {
            pMorphData->ShapeData1.EndShape();
            pMorphData->ShapeData2.EndShape();
            break;
        }

        SF_ASSERT(pathType1 == pathType2);
        if (pathType1 == Shape_NewLayer || first)
        {
            pMorphData->ShapeData1.StartLayer();
            pMorphData->ShapeData2.StartLayer();
            first = false;
        }
        pMorphData->ShapeData1.StartPath(styles1[0], styles1[1], styles1[2]);
        pMorphData->ShapeData2.StartPath(styles1[0], styles1[1], styles1[2]); // Yes, styles1 !!!

        pMorphData->ShapeData1.MoveTo(coord1[0], coord1[1]);
        pMorphData->ShapeData2.MoveTo(coord2[0], coord2[1]);

        start1[0] = coord1[0]; start1[1] = coord1[1]; 
        start2[0] = coord2[0]; start2[1] = coord2[1]; 

        for(;;)
        {
            ShapePosInfo prevPos1 = posInfo1;
            ShapePosInfo prevPos2 = posInfo2;

            edgeType1 = morph1->ReadEdge(&posInfo1, coord1);
            edgeType2 = morph2->ReadEdge(&posInfo2, coord2);

            if (edgeType1 == Edge_EndPath && edgeType2 == Edge_EndPath)
            {
                pMorphData->ShapeData1.EndPath();
                pMorphData->ShapeData2.EndPath();
                break;
            }
            else
            if (edgeType1 == Edge_EndPath)
            {
                posInfo2 = prevPos2;
                pMorphData->ShapeData1.EndPath();
                pMorphData->ShapeData2.EndPath();
                pathType1 = morph1->ReadPathInfo(&posInfo1, coord1, styles1);

                //SF_ASSERT(pathType1 == Shape_NewPath);
                if (pathType1 == Shape_EndShape)
                {
                    pMorphData->ShapeData1.EndShape();
                    pMorphData->ShapeData2.EndShape();
                    done = true;
                    break;
                }

                pMorphData->ShapeData1.StartPath(styles1[0], styles1[1], styles1[2]);
                pMorphData->ShapeData2.StartPath(styles1[0], styles1[1], styles1[2]); // Yes, styles1 !!!
                pMorphData->ShapeData1.MoveTo(coord1[0], coord1[1]);
                pMorphData->ShapeData2.MoveTo(start2[0], start2[1]);
                start1[0] = coord1[0]; start1[1] = coord1[1]; 
            }
            else
            if (edgeType2 == Edge_EndPath)
            {
                posInfo1 = prevPos1;
                pMorphData->ShapeData1.EndPath();
                pMorphData->ShapeData2.EndPath();
                pathType2 = morph2->ReadPathInfo(&posInfo2, coord2, styles2);

                //SF_ASSERT(pathType2 == Shape_NewPath);
                if (pathType2 == Shape_EndShape)
                {
                    pMorphData->ShapeData1.EndShape();
                    pMorphData->ShapeData2.EndShape();
                    done = true;
                    break;
                }

                pMorphData->ShapeData1.StartPath(styles1[0], styles1[1], styles1[2]);
                pMorphData->ShapeData2.StartPath(styles1[0], styles1[1], styles1[2]); // Yes, styles1 !!!
                pMorphData->ShapeData1.MoveTo(start1[0], start1[1]);
                pMorphData->ShapeData2.MoveTo(coord2[0], coord2[1]);
                start2[0] = coord2[0]; start2[1] = coord2[1]; 
            }
            else
            {
                SF_ASSERT(edgeType1 != Edge_CubicTo || edgeType1 == edgeType2);
                if (edgeType1 != edgeType2)
                {
                    if (edgeType1 == Edge_LineTo && edgeType2 == Edge_QuadTo)
                    {
                        coord1[2] = coord1[0];
                        coord1[3] = coord1[1];
                        coord1[0] = (start1[0] + coord1[2]) / 2;
                        coord1[1] = (start1[1] + coord1[3]) / 2;
                        edgeType1 = Edge_QuadTo;
                    }
                    if (edgeType1 == Edge_QuadTo && edgeType2 == Edge_LineTo)
                    {
                        coord2[2] = coord2[0];
                        coord2[3] = coord2[1];
                        coord2[0] = (start2[0] + coord2[2]) / 2;
                        coord2[1] = (start2[1] + coord2[3]) / 2;
                        edgeType2 = Edge_QuadTo;
                    }
                }

                if (edgeType1 == Edge_LineTo)
                {
                    pMorphData->ShapeData1.LineTo(coord1[0], coord1[1]);
                    pMorphData->ShapeData2.LineTo(coord2[0], coord2[1]);
                    start1[0] = coord1[0]; start1[1] = coord1[1]; 
                    start2[0] = coord2[0]; start2[1] = coord2[1]; 
                }
                else
                if (edgeType1 == Edge_QuadTo)
                {
                    pMorphData->ShapeData1.QuadTo(coord1[0], coord1[1], coord1[2], coord1[3]);
                    pMorphData->ShapeData2.QuadTo(coord2[0], coord2[1], coord2[2], coord2[3]);
                    start1[0] = coord1[2]; start1[1] = coord1[3]; 
                    start2[0] = coord2[2]; start2[1] = coord2[3]; 
                }
                else
                if (edgeType1 == Edge_CubicTo)
                {
                    pMorphData->ShapeData1.CubicTo(coord1[0], coord1[1], coord1[2], coord1[3], coord1[4], coord1[5]);
                    pMorphData->ShapeData2.CubicTo(coord2[0], coord2[1], coord2[2], coord2[3], coord2[4], coord2[5]);
                    start1[0] = coord1[4]; start1[1] = coord1[5]; 
                    start2[0] = coord2[4]; start2[1] = coord2[5]; 
                }
            }
        }
    }
}



//------------------------------------------------------------------------
void ShapeMeshProvider::countComplexFills(const TmpPathsArray& paths, UPInt i1, UPInt i2, DrawLayerType* dl)
{
    UPInt i;
    BitSet fills(Memory::GetGlobalHeap());
    unsigned style;
    unsigned complexFill;
    dl->StartFill = (unsigned)FillToStyleTable.GetSize();
    dl->FillCount = 0;
    for (i = i1; i < i2; ++i)
    {
        if (paths[i].Styles[0] != paths[i].Styles[1])
        {
            FillStyleType fill;
            style = paths[i].Styles[0];
            if (style)
            {
                pShapeData->GetFillStyle(style, &fill);
                complexFill = fill.pFill.GetPtr() ? style : 0;
                if (fills[complexFill] == 0)
                {
                    FillToStyleTable.PushBack(complexFill);
                    dl->FillCount++;
                    fills.Set(complexFill);
                }
            }
            style = paths[i].Styles[1];
            if (style)
            {
                pShapeData->GetFillStyle(style, &fill);
                complexFill = fill.pFill.GetPtr() ? style : 0;
                if (fills[complexFill] == 0)
                {
                    FillToStyleTable.PushBack(complexFill);
                    dl->FillCount++;
                    fills.Set(complexFill);
                }
            }
        }
    }
}


//------------------------------------------------------------------------
void ShapeMeshProvider::createDrawLayers(const TmpPathsArray& paths, UPInt i1, UPInt i2)
{
    if (i2 > i1)
    {
        DrawLayerType dl;
        BitSet styles(Memory::GetGlobalHeap());
        unsigned startPos = paths[i1].Pos;
        UPInt i;

        // Set fill layer and count fills
        //------------------------
        for (i = i1; i < i2; ++i)
        {
            if (paths[i].Styles[0] != paths[i].Styles[1])   // Fill layer
            {
                if (styles[0] == 0)
                {
                    countComplexFills(paths, i1, i2, &dl);
                    dl.StartPos    = startPos;
                    dl.StrokeStyle = 0;
                    dl.Image9GridType = I9gNone;
                    DrawLayers.PushBack(dl);
                    styles.Set(0);
                }
            }

            if (paths[i].Styles[2])                         // Stroke layer
            {
                styles.Set(paths[i].Styles[2]);
            }
        }

        // Count stroke layers
        //------------------------
        for (i = i1; i < i2; ++i)
        {
            unsigned style = paths[i].Styles[2];
            if (style && styles[style])
            {
                dl.FillCount   = 1;     // For strokes the fill is calculated directly,
                                        // which saves some space for multi-stroke shapes.
                dl.StartFill   = 0;
                dl.StartPos    = startPos;
                dl.StrokeStyle = style;
                dl.Image9GridType = I9gNone;
                DrawLayers.PushBack(dl);
                styles.Clr(style);
                Strokes = true;
                SF_AMP_CODE(AmpServer::GetInstance().AddStrokes(1);)
            }
        }
    }
}

//------------------------------------------------------------------------
void ShapeMeshProvider::acquireShapeData()
{
    SF_ASSERT(pShapeData.GetPtr());

    const ShapeDataInterface* shape = pMorphData.GetPtr() ? &pMorphData->ShapeData1 : pShapeData;

    unsigned startPos = shape->GetStartingPos();
    ShapePosInfo posInfo(startPos);

    ShapePathType pathType;
    float coords[Edge_MaxCoord];
    unsigned styles[3];
    bool first = true;
    unsigned shapeLayer = 0;

    ArrayStaticBuffPOD<TmpPathInfoType, 32> paths(Memory::GetGlobalHeap());
    Strokes = false;

    // Collect the paths
    //-------------------------
    for(;;)
    {
        unsigned prevPos = posInfo.Pos;
        pathType = shape->ReadPathInfo(&posInfo, coords, styles);
        if (pathType == Shape_EndShape)
            break;

        if (pathType == Shape_NewLayer && !first)
            ++shapeLayer;

        TmpPathInfoType path;
        path.Pos        = prevPos;
        path.ShapeLayer = shapeLayer;
        path.Styles[0]  = styles[0];
        path.Styles[1]  = styles[1];
        path.Styles[2]  = styles[2];
        path.EdgeCount  = shape->CountPathEdges(&posInfo);
        paths.PushBack(path);
        first = false;
    }

    // Remove all empty paths
    //-------------------------
    UPInt i;
    UPInt j = 0;
    for(i = 0; i < paths.GetSize(); ++i)
    {
        if (paths[i].EdgeCount && (paths[i].Styles[0] || paths[i].Styles[1] || paths[i].Styles[2]))
        {
            paths[j++] = paths[i];
        }
    }
    paths.CutAt(j);

    DrawLayers.Clear();
    SF_AMP_CODE(clearStrokeCount();)
    FillToStyleTable.Clear();

    // Create Draw Layers and count complex fills
    //-------------------------
    shapeLayer = 0;
    UPInt prevIdx = 0;
    for(i = 0; i < paths.GetSize(); ++i)
    {
        if (shapeLayer != paths[i].ShapeLayer)
        {
            createDrawLayers(paths, prevIdx, i);
            prevIdx = i;
            shapeLayer = paths[i].ShapeLayer;
        }
    }
    createDrawLayers(paths, prevIdx, paths.GetSize());
    IdentityBounds = GetBounds(Matrix2F());


    // Identify possible Image9GridType. At this moment it's not known
    // whether or not scale9 is actually used. It's only a flag that allows
    // for automatic slicing.
    //-------------------------
    if (checkI9gMergedSlice())
    {
        // In case merging compatible scale9grid is detected it always will be 
        // drawn as scale9grid even is scale9grid is not actually set.
        DrawLayers.Resize(1);
        SF_AMP_CODE(clearStrokeCount());
        DrawLayers[0].Image9GridType = I9gMergedSlice;
    }
    else
    {
        perceiveImage9GridLayers();
    }
}


//------------------------------------------------------------------------
bool ShapeMeshProvider::checkI9gMergedSlice() const
{
    if (pMorphData.GetPtr())
        return false; // Shape morph is not supported with Image Scale9

    // Merging compatible image scale9grid generated by  Grant Skinner's 
    // JSFL command. See http://www.gskinner.com/blog/archives/2010/04/bitmapslice9_sc.html
    // The plug-in generates 9 rectangles in one shape, separated by NewShape flag.
    // The conditions are:
    // - the shape must have exactly 9 image fill styles and 0 stroke styles
    // - the shape must have exactly 9 layers
    // - all images in the fill styles must be the same
    // - the sum area of all 9 rectangles must be equal to the area 
    //   of the bounding box, with 2x2 pixels tolerance.
    //--------------------------------------
    unsigned imgStyleCount = 0;
    if (DrawLayers.GetSize() == 9 && GetFillStyleCount() == 9 && GetStrokeStyleCount() == 0)
    {
        const Image* img = 0;
        for(unsigned i = 0; i < 9; ++i)
        {
            FillStyleType f;
            pShapeData->GetFillStyle(i+1, &f);
            if (f.pFill.GetPtr() == 0)
                return false;
            if (f.pFill->pImage.GetPtr() != 0 && (img == 0 || img == f.pFill->pImage.GetPtr()))
                imgStyleCount++;

            img = f.pFill->pImage.GetPtr();
        }
    }
    if (imgStyleCount != 9)
        return false;

    // TO DO: Possibly check for geometry (9 rects and area)

    return true;
}


//------------------------------------------------------------------------
bool ShapeMeshProvider::checkI9gLayer(const DrawLayerType& dl) const
{
    if (pMorphData.GetPtr())
        return false; // Shape morph is not supported with Image Scale9

    ShapePosInfo posInfo(dl.StartPos);
    ShapePathType pathType;
    float coords[Edge_MaxCoord];
    unsigned styles[3];
    pathType = pShapeData->ReadPathInfo(&posInfo, coords, styles);
    if ((pathType == Shape_NewLayer || pathType == Shape_NewPath) && 
        (styles[0] == 0) != (styles[1] == 0) && styles[2] == 0)
    {
        unsigned style = styles[0] ? styles[0] : styles[1];
        FillStyleType f;
        pShapeData->GetFillStyle(style, &f);
        if (f.pFill.GetPtr() != 0 && f.pFill->pImage.GetPtr() != 0 && 
           !f.pFill->ImageMatrix.IsFreeRotation() &&
            f.pFill->FillMode.GetWrapMode() == Wrap_Clamp)
        {
            pShapeData->SkipPathData(&posInfo);
            pathType = pShapeData->ReadPathInfo(&posInfo, coords, styles);
            if (pathType == Shape_NewLayer || pathType == Shape_EndShape)
            {
                // TO DO: Possibly check for geometry (rectangle)
                return true;
            }
        }
    }
    return false;
}

//------------------------------------------------------------------------
void ShapeMeshProvider::perceiveImage9GridLayers()
{
    for(UPInt i = 0; i < DrawLayers.GetSize(); ++i)
    {
        DrawLayerType& dl = DrawLayers[i];
        if (dl.StrokeStyle == 0 && checkI9gLayer(dl))
        {
            dl.Image9GridType = I9gNormalSlice;
        }
    }
}

//------------------------------------------------------------------------
bool fillStyleIsComplex(const ShapeDataInterface* shape, unsigned style)
{
    if (style)
    {
        FillStyleType f;
        shape->GetFillStyle(style, &f);
        return f.pFill.GetPtr() != 0;
    }
    return false;
}

//------------------------------------------------------------------------
void ShapeMeshProvider::GetFillStyle(unsigned idx, FillStyleType* f1, float morphRatio) const 
{ 
//printf("F"); // DBG
    pShapeData->GetFillStyle(idx, f1);
    if (pMorphData.GetPtr() == 0 || morphRatio == 0)
        return;

    FillStyleType f2;
    pMorphData->pMorphTo->GetFillStyle(idx, &f2);

    if (f1->pFill.GetPtr() == 0)
    {
        f1->Color = Color::Blend(Color(f1->Color), Color(f2.Color), morphRatio).Raw;
        return;
    }
}

//------------------------------------------------------------------------
void ShapeMeshProvider::GetStrokeStyle(unsigned idx, StrokeStyleType* s1, float morphRatio) const 
{ 
    pShapeData->GetStrokeStyle(idx, s1); 
    if (pMorphData.GetPtr() == 0 || morphRatio == 0)
        return;

    StrokeStyleType s2;
    pMorphData->pMorphTo->GetStrokeStyle(idx, &s2);

    s1->Width = Alg::Lerp(s1->Width, s2.Width, morphRatio);

    if (s1->pFill.GetPtr() == 0)
    {
        s1->Color = Color::Blend(Color(s1->Color), Color(s2.Color), morphRatio).Raw;
        return;
    }
}


//------------------------------------------------------------------------
void ShapeMeshProvider::addFill(MeshGenerator* gen, const ToleranceParams& param,
                                const TransformerBase* tr, unsigned startPos, 
                                float morphRatio)
{
//gen->mTess.Clear();
//unsigned nv=0;
//unsigned np=0;
//static unsigned nn=0;
//FILE* fd = fopen("shapes", "at");
//FILE* fd2 = fopen("inc", "at");
//fprintf(fd, "=======BeginShape\n");

    ShapePosInfo pos1(startPos);
    ShapePosInfo pos2(startPos);
    ShapePathType pathType;
    PathEdgeType pathEdge;
    float coord[Edge_MaxCoord];
    unsigned styles[3];
    bool first = true;
    MorphInterpolator shape(pShapeData, pMorphData, morphRatio, pos2);

    while((pathType = shape.ReadPathInfo(&pos1, coord, styles)) != Shape_EndShape)
    {
        if (!first && pathType == Shape_NewLayer)
            break;
        first = false;

        if (styles[0] != styles[1])
        {
//fprintf(fd, "Path %u %u %u %f %f\n", styles[0], styles[1], styles[2], coord[0], coord[1]);
            tr->Transform(&coord[0], &coord[1]);
            gen->mTess.AddVertex(coord[0], coord[1]);
//++nv;
//fprintf(fd2, "%u %u\n", nv, gen->mTess.GetSrcVertexCount());
            while((pathEdge = shape.ReadEdge(&pos1, coord)) != Edge_EndPath)
            {
                if (pathEdge == Edge_LineTo)
                {
//fprintf(fd, "Line %f %f\n", coord[0], coord[1]);
                    tr->Transform(&coord[0], &coord[1]);
                    gen->mTess.AddVertex(coord[0], coord[1]);
//++nv;
//fprintf(fd2, "%u %u\n", nv, gen->mTess.GetSrcVertexCount());
                }
                else
                if (pathEdge == Edge_QuadTo)
                {
//fprintf(fd, "Curve %f %f %f %f\n", coord[0], coord[1], coord[2], coord[3]);
                    tr->Transform(&coord[0], &coord[1]);
                    tr->Transform(&coord[2], &coord[3]);
                    TessellateQuadCurve(&gen->mTess, param, coord[0], coord[1], coord[2], coord[3]);
//++nv;
//fprintf(fd2, "%u %u\n", nv, gen->mTess.GetSrcVertexCount());
                }
                else
                if (pathEdge == Edge_CubicTo)
                {
//fprintf(fd, "Cubic %f %f %f %f %f %f\n", coord[0], coord[1], coord[2], coord[3], coord[4], coord[5]);
                    tr->Transform(&coord[0], &coord[1]);
                    tr->Transform(&coord[2], &coord[3]);
                    tr->Transform(&coord[4], &coord[5]);
                    TessellateCubicCurve(&gen->mTess, param, coord[0], coord[1], coord[2], coord[3], coord[4], coord[5]);
//++nv;
//fprintf(fd2, "%u %u\n", nv, gen->mTess.GetSrcVertexCount());
                }
            }
//fprintf(fd, "<-------EndPath\n");
//fprintf(fd2, "-- %d ", gen->mTess.GetSrcVertexCount());
            gen->mTess.FinalizePath(styles[0], styles[1], 
                                    fillStyleIsComplex(pShapeData, styles[0]), 
                                    fillStyleIsComplex(pShapeData, styles[1]));
//fprintf(fd2, "-- %d\n", gen->mTess.GetSrcVertexCount());
//++np;
        }
        else
        {
            shape.SkipPathData(&pos1);
        }
    }
//printf("[ nn=%u np=%u %u %u ] ", nn, np, nv, gen->mTess.GetSrcVertexCount());
//fprintf(fd, "!======EndShape\n");
//++nn;
//fclose(fd2);
//fclose(fd);
}


//------------------------------------------------------------------------
void ShapeMeshProvider::addToStrokeSorter(MeshGenerator* gen,
                                          const ToleranceParams& param,
                                          const TransformerBase* tr, unsigned startPos,
                                          unsigned strokeStyleIdx, float snapOffset,
                                          float morphRatio)
{
    ShapePosInfo pos1(startPos);
    ShapePosInfo pos2(startPos);
    ShapePathType pathType;
    PathEdgeType pathEdge;
    float coord[Edge_MaxCoord];
    unsigned styles[3];

    StrokeStyleType style;
    GetStrokeStyle(strokeStyleIdx, &style, morphRatio);

    MorphInterpolator shape(pShapeData, pMorphData, morphRatio, pos2);

    bool first = true;
    while((pathType = shape.ReadPathInfo(&pos1, coord, styles)) != Shape_EndShape)
    {
        if (!first && pathType == Shape_NewLayer)
            break;
        first = false;

        if (styles[2] == strokeStyleIdx)
        {
            gen->mStrokeSorter.AddVertexNV(coord[0], coord[1]);
            while((pathEdge = shape.ReadEdge(&pos1, coord)) != Edge_EndPath)
            {
                if (pathEdge == Edge_LineTo)
                {
                    gen->mStrokeSorter.AddVertexNV(coord[0], coord[1]);
                }
                else
                if (pathEdge == Edge_QuadTo)
                {
                    gen->mStrokeSorter.AddQuad(coord[0], coord[1], coord[2], coord[3]);
                }
                else
                if (pathEdge == Edge_CubicTo)
                {
                    gen->mStrokeSorter.AddCubic(coord[0], coord[1], coord[2], coord[3], coord[4], coord[5]);
                }
            }
            gen->mStrokeSorter.FinalizePath(0, 0, false, false);
        }
        else
        {
            shape.SkipPathData(&pos1);
        }
    }
    gen->mStrokeSorter.Sort();

    if (style.pDashes)
    {
        gen->mStrokeSorter.GenerateDashes(style.pDashes, param, tr->GetScale());
    }

    gen->mStrokeSorter.Transform(tr);

    if (style.Flags & StrokeFlag_StrokeHinting)
    {
        // The dilemma here is that we can't snap tessellated curves.
        // In case of using dash generator the source curves are already lost, 
        // we only have line segments. Snapping line segments produce ugly result.
        if (style.pDashes.GetPtr() == 0)
            gen->mStrokeSorter.Snap(snapOffset, snapOffset);
    }
    else
    if (snapOffset > 0)
    {
        gen->mStrokeSorter.AddOffset(snapOffset, snapOffset);    
    }
}


//------------------------------------------------------------------------
void ShapeMeshProvider::addStroke(MeshGenerator* gen,
                                  TessBase* stroker,
                                  const ToleranceParams& param,
                                  const TransformerBase* tr, unsigned startPos, 
                                  unsigned strokeStyleIdx, float snapOffset, 
                                  float morphRatio)
{
    addToStrokeSorter(gen, param, tr, startPos, strokeStyleIdx, snapOffset, morphRatio);
    stroker->Clear();

    unsigned i, j;
    for (i = 0; i < gen->mStrokeSorter.GetPathCount(); ++i)
    {
        unsigned n = gen->mStrokeSorter.GetVertexCount(i);
        for (j = 0; j < n; )
        {
            const StrokeSorter::VertexType& v1 = gen->mStrokeSorter.GetVertex(i, j++);
            switch(v1.segType)
            {
            case Math2D::Seg_LineTo:
                {
                    stroker->AddVertex(v1.x, v1.y);
                }
                break;

            case Math2D::Seg_QuadTo:
                {
                    const StrokeSorter::VertexType& v2 = gen->mStrokeSorter.GetVertex(i, j++);
                    TessellateQuadCurve(stroker, param, v1.x, v1.y, v2.x, v2.y);
                }
                break;

            case Math2D::Seg_CubicTo:
                {
                    const StrokeSorter::VertexType& v2 = gen->mStrokeSorter.GetVertex(i, j++);
                    const StrokeSorter::VertexType& v3 = gen->mStrokeSorter.GetVertex(i, j++);
                    TessellateCubicCurve(stroker, param, v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
                }
                break;
            }
        }
        stroker->FinalizePath(0, 0, false, false);
    }
}

//------------------------------------------------------------------------
void ShapeMeshProvider::addStroke(MeshGenerator* gen,
                                  const ToleranceParams& param,
                                  const TransformerBase* tr, unsigned startPos, 
                                  unsigned strokeStyleIdx, float snapOffset, 
                                  float morphRatio)
{
    addToStrokeSorter(gen, param, tr, startPos, strokeStyleIdx, snapOffset, morphRatio);
    gen->mStroker.Clear();

    unsigned i, j;
    for (i = 0; i < gen->mStrokeSorter.GetPathCount(); ++i)
    {
        unsigned n = gen->mStrokeSorter.GetVertexCount(i);
        for (j = 0; j < n; )
        {
            const StrokeSorter::VertexType& v1 = gen->mStrokeSorter.GetVertex(i, j++);
            switch(v1.segType)
            {
            case Math2D::Seg_LineTo:
                {
                    gen->mStroker.AddVertex(v1.x, v1.y);
                }
                break;

            case Math2D::Seg_QuadTo:
                {
                    const StrokeSorter::VertexType& v2 = gen->mStrokeSorter.GetVertex(i, j++);
                    TessellateQuadCurve(&gen->mStroker, param, v1.x, v1.y, v2.x, v2.y);
                }
                break;

            case Math2D::Seg_CubicTo:
                {
                    const StrokeSorter::VertexType& v2 = gen->mStrokeSorter.GetVertex(i, j++);
                    const StrokeSorter::VertexType& v3 = gen->mStrokeSorter.GetVertex(i, j++);
                    TessellateCubicCurve(&gen->mStroker, param, v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
                }
                break;
            }
        }
        gen->mStroker.GenerateStroke(&gen->mTess);
    }
}


//------------------------------------------------------------------------
bool ShapeMeshProvider::createNullMesh(VertexOutput* pout, unsigned drawLayer, unsigned meshGenFlags)
{
    SF_UNUSED2(drawLayer, meshGenFlags);
    VertexXY16iC32 ver[1];
    UInt16 tri[3];
    ver[0].x = 0; 
    ver[0].y = 0; 
    ver[0].Color = 0; 
    tri[0] = tri[1] = tri[2] = 0;
  
    FillData fd;
    //GetFillData(&fd, 0, 0, 0);
    VertexOutput::Fill vfill = { 1, 3, fd.pVFormat, 0,0,0 };
    //VertexOutput::Fill vfill = { 1, 3, 0, 0,0,0 };

    if (!pout->BeginOutput(&vfill, 1))
        return false; 

    pout->SetVertices(0, 0, ver, 1);
    pout->SetIndices(0, 0, tri, 3);
    pout->EndOutput();
    return true;
}


//------------------------------------------------------------------------
bool ShapeMeshProvider::acquireTessMeshes(TessBase* tess, const Matrix2F& mtx, VertexOutput* pout,
                                          unsigned drawLayerIdx, unsigned strokeStyleIdx,
                                          unsigned meshGenFlags, float morphRatio)
{
    VertexXY16iCF32     meshVerC[VerBufSize];
    //VertexXY16iC32      meshVerC[VerBufSize];
    TessVertex          tessVer[VerBufSize];
    UInt16              tri[TriBufSize * 3];
    TessMesh            tessMesh;
    StrokeStyleType     strokeStyle;
    // Use global/render heap for temp store; ShapeMeshProvider heap must not be used here.
    ArrayStaticBuffPOD<VertexOutput::Fill, 16> fills(Memory::GetGlobalHeap());
    ArrayStaticBuffPOD<unsigned, 16> fillIndexByStyle(Memory::GetGlobalHeap());

    if (tess->GetMeshCount() == 0 || tess->GetVertexCount() == 0)
    {
        return createNullMesh(pout, drawLayerIdx, meshGenFlags);
    }

    if (strokeStyleIdx == 0)
    {
        unsigned i;
        unsigned styleCount = GetFillStyleCount() + 1; // The styles are in range [1...FillStyleCount] (inclusive).
        for (i = 0; i < styleCount; ++i)
            fillIndexByStyle.PushBack(0);

        unsigned fillCount = GetFillCount(drawLayerIdx, meshGenFlags);
        for (i = 0; i < fillCount; ++i)
        {
            unsigned fill = GetStyleByFillIdx(drawLayerIdx, i);
            SF_ASSERT(fill < fillIndexByStyle.GetSize());
            fillIndexByStyle[fill] = i;
        }
    }
    else
    {
        GetStrokeStyle(strokeStyleIdx, &strokeStyle, morphRatio);
    }

    tess->Transform(mtx.GetInverse());

    // TO DO: Consider float vertex format
    Matrix m16i = tess->StretchTo(-32768+4, -32768+4, 32768-4, 32768-4);

    unsigned meshIdx;
    unsigned totalVer = 0;
    for (meshIdx = 0; meshIdx < tess->GetMeshCount(); ++meshIdx)
    {
        VertexOutput::Fill vfill;

        tess->GetMesh(meshIdx, &tessMesh);

        vfill.VertexCount = tess->GetMeshVertexCount(meshIdx);
        vfill.IndexCount  = tess->GetMeshTriangleCount(meshIdx) * 3;
        if (vfill.VertexCount && vfill.IndexCount)
        {
            vfill.pFormat     = &VertexXY16iCF32::Format; // TO DO: Consider other formats (XY,Color)
            vfill.FillIndex0  = 0;
            vfill.FillIndex1  = 0;
            if (strokeStyleIdx == 0)
            {
                vfill.FillIndex0 = fillIndexByStyle[tessMesh.Style1];
                vfill.FillIndex1 = fillIndexByStyle[tessMesh.Style2];
            }
            vfill.MergeFlags = FillMerge_EAlpha; // TO DO: Consider FillMerge_None when there's no Cxform
            if (TessStyleIsComplex(tessMesh.Flags1))
                vfill.MergeFlags |= FillMerge_Weight;

            vfill.MeshIndex = meshIdx;

            fills.PushBack(vfill);
            totalVer += tessMesh.VertexCount;
        }
    }

    if (fills.GetSize() == 0)
    {
        return createNullMesh(pout, drawLayerIdx, meshGenFlags);
    }

    if (totalVer > 65535)
    {
        Log* log = Log::GetGlobalLog();
        if (log == 0)
            log = Log::GetDefaultLog();
        if (log)
            log->LogWarning("Render_ShapeMeshProvider: More than 65535 vertices, the shape cannot be displayed");
        return createNullMesh(pout, drawLayerIdx, meshGenFlags);
    }

    if (!pout->BeginOutput(&fills[0], (unsigned)fills.GetSize(), m16i))
        return false;

    // TO DO: Optimize style lerp (pre-calc morph styles) 

    unsigned fillIdx;
    unsigned prevFill1 = 0;
    unsigned prevFill2 = 0;
    FillStyleType f1, f2;
    for (fillIdx = 0; fillIdx < fills.GetSize(); ++fillIdx)
    {
        const VertexOutput::Fill& vfill = fills[fillIdx];
        tess->GetMesh(vfill.MeshIndex, &tessMesh);

        unsigned start = 0;
        unsigned retVer;
        while((retVer = tess->GetVertices(&tessMesh, tessVer, VerBufSize)) > 0)
        {
            unsigned j;
            for(j = 0; j < retVer; ++j)
            {
                const TessVertex& tv = tessVer[j];
                VertexXY16iCF32&  mv = meshVerC[j];
                mv.x = (SInt16)floorf((tv.x < 0) ? tv.x-0.5f : tv.x+0.5f);
                mv.y = (SInt16)floorf((tv.y < 0) ? tv.y-0.5f : tv.y+0.5f);
                if (strokeStyleIdx)
                {
                    mv.Color = strokeStyle.Color;
                }
                else
                {
                    mv.Color = 0;
                    if (!TessStyleIsComplex(tv.Flags))
                    {
                        if (TessStyleMixesColors(tv.Flags))
                        {
                            if (prevFill1 != tv.Styles[0])
                            {
                                GetFillStyle(tv.Styles[0], &f1, morphRatio);
                                prevFill1 = tv.Styles[0];
                            }
                            if (prevFill2 != tv.Styles[1])
                            {
                                GetFillStyle(tv.Styles[1], &f2, morphRatio);
                                prevFill2 = tv.Styles[1];
                            }
                            mv.Color = ((f1.Color & 0xFEFEFEFE) >> 1) | ((f2.Color & 0xFEFEFEFE) >> 1);
                        }
                        else
                        {
                            if (prevFill1 != tv.Styles[TessGetUsedStyle(tv.Flags)])
                            {
                                GetFillStyle(tv.Styles[TessGetUsedStyle(tv.Flags)], &f1, morphRatio);
                                prevFill1 = tv.Styles[TessGetUsedStyle(tv.Flags)];
                            }
                            mv.Color = f1.Color;
                        }
                    }
                }
                mv.Factors[0] = Factors[TessGetAlpha(tv.Flags)];
                mv.Factors[1] = Factors[TessGetFactor(tv.Flags)];
            }
            pout->SetVertices(fillIdx, start, meshVerC, retVer);
            start += retVer;
        }

        start = 0;
        unsigned nTri = tess->GetMeshTriangleCount(vfill.MeshIndex);
        while (start < nTri)
        {
            unsigned retTri = TriBufSize;
            if (start + retTri > nTri)
            {
                retTri = nTri - start;
                if (retTri == 0) break;
            }
            tess->GetTrianglesI16(vfill.MeshIndex, tri, start, retTri);
            pout->SetIndices(fillIdx, start*3, tri, retTri*3);
            start += retTri;
        }
    }

    pout->EndOutput();
    return true;
}

//------------------------------------------------------------------------
bool ShapeMeshProvider::tessellateFill(const Scale9GridInfo* s9g, unsigned drawLayerIdx, 
                                       MeshBase *pmesh, VertexOutput* pout,
                                       unsigned meshGenFlags)
{
//float morphRatio = pmesh->GetMorphRatio(); // DBG
//printf("\n%f ", morphRatio);

    Renderer2DImpl* ren = pmesh->GetRenderer();
    MeshGenerator*  gen = ren->GetMeshGen();
    const Matrix2F& mtx = pmesh->GetViewMatrix(); // tbd: Better passed as argument.
    float           morphRatio = pmesh->GetMorphRatio();
    ToleranceParams param = ren->GetToleranceParams();

    TransformerBase* tr = 0;
    TransformerWrapper<Matrix2F> trAffine;
    TransformerWrapper<Scale9GridInfo> trScale9;

    Matrix2F m2;
    if (s9g)
    {
        m2 = mtx;
        trScale9 = TransformerWrapper<Scale9GridInfo>(s9g);
        tr = &trScale9;
    }
    else
    {
        SF_SIMD_ALIGN( float key[MeshKey::MaxKeySize] );
        bool mtxValid = MeshKey::CalcMatrixKey(mtx, key, &m2);
        if (!mtxValid)
        {
            return createNullMesh(pout, drawLayerIdx, meshGenFlags);
        }
        trAffine = TransformerWrapper<Matrix2F>(&m2);
        tr = &trAffine;
    }

    bool success = false;
    for(unsigned i = 0; i < 4; ++i) // TO DO: Revise/Remove. Hack to increase tolerance
    {
        gen->Clear();
        gen->mTess.SetFillRule(Tessellator::FillEvenOdd);
        gen->mTess.SetToleranceParam(param);
#ifdef SF_RENDER_ENABLE_EDGEAA
        if ((meshGenFlags & Mesh_EdgeAA) != 0 && (meshGenFlags & Mesh_Mask) == 0)
        {
            gen->mTess.SetEdgeAAWidth(0.5f * param.EdgeAAScale);
        }
        else
        {
            gen->mTess.SetEdgeAAWidth(0);
        }
#endif
        addFill(gen, param, tr, GetLayerStartPos(drawLayerIdx), morphRatio);

        // TO DO: Revise/Remove. Hack to turn off EdgeAA
        if (meshGenFlags & Mesh_EdgeAA)
        {
#ifdef SF_RENDER_ENABLE_EDGEAA
            unsigned srcVer = gen->mTess.GetSrcVertexCount();
            if (srcVer >= 65536/2)
            {
                gen->Clear();
                gen->mTess.SetEdgeAAWidth(0);
                addFill(gen, param, tr, GetLayerStartPos(drawLayerIdx), morphRatio);
                meshGenFlags &= ~Mesh_EdgeAA;
            }
#endif
        }

        gen->mTess.Tessellate(false);
        if(gen->mTess.GetVertexCount() < 65535)
        {
            success = true;
                break;
        }
        meshGenFlags &= ~Mesh_EdgeAA;
        param.CurveTolerance *= 4;
    }

//printf("*"); // DBG

    if (success)
    {
        bool ret = acquireTessMeshes(&gen->mTess, m2, pout, drawLayerIdx, 0, meshGenFlags, morphRatio);
        gen->Clear();
        return ret;
    }
    gen->Clear();
    return createNullMesh(pout, drawLayerIdx, meshGenFlags);
}


//------------------------------------------------------------------------
bool ShapeMeshProvider::tessellateStroke(const Scale9GridInfo* s9g, 
                                         unsigned strokeStyleIdx, unsigned drawLayerIdx,
                                         MeshBase *pmesh, VertexOutput* pout,
                                         unsigned meshGenFlags)
{
    Renderer2DImpl* ren = pmesh->GetRenderer();
    MeshGenerator*  gen = ren->GetMeshGen();
    Matrix2F        mtx = pmesh->GetViewMatrix(); // tbd: Better passed as argument.
    float           morphRatio = pmesh->GetMorphRatio();
    const ToleranceParams& param = ren->GetToleranceParams();
    StrokeStyleType strokeStyle;
    GetStrokeStyle(strokeStyleIdx, &strokeStyle, pmesh->GetMorphRatio());
    unsigned startPos = GetLayerStartPos(drawLayerIdx);

    if (strokeStyle.Miter < 1.0f) // Protect from incorrect stroke calculation
        strokeStyle.Miter = 1.0f; // TO DO: Revise (consider ASSERT)

    // TO DO: revise, consider offset in the MeshKey
    bool hinted = (strokeStyle.Flags & StrokeFlag_StrokeHinting) != 0;
    if (hinted)
    {
        mtx.Tx() = floorf(mtx.Tx() + ((mtx.Tx() < 0) ? -0.5f : +0.5f));
        mtx.Ty() = floorf(mtx.Ty() + ((mtx.Ty() < 0) ? -0.5f : +0.5f));
    }

    Matrix2F m2 = mtx;

    TransformerBase* tr = 0;
    TransformerWrapper<Matrix2F> trAffine;
    TransformerWrapper<Scale9GridInfo> trScale9;

    if (s9g)
    {
        trScale9 = TransformerWrapper<Scale9GridInfo>(s9g);
        tr = &trScale9;
    }
    else
    {
        trAffine = TransformerWrapper<Matrix2F>(&m2);
        tr = &trAffine;
    }

    gen->Clear();

    bool ret = true;
    float widthScale = strokeStyle.Units;
    if (s9g == 0)
    {
        switch(strokeStyle.Flags & StrokeScaling_Mask)
        {
        case StrokeScaling_Normal:      widthScale = mtx.GetScale();  break;
        case StrokeScaling_Horizontal:  widthScale = mtx.GetXScale(); break;
        case StrokeScaling_Vertical:    widthScale = mtx.GetYScale(); break;
        case StrokeScaling_None:                                      break;
        }
    }

    float screenWidth = strokeStyle.Width * widthScale;
    float edgeAAWidth = 0.5f;
    float offset = 0.5f;

    if ((meshGenFlags & Mesh_EdgeAA) == 0 || (meshGenFlags & Mesh_Mask) != 0)
    {
        edgeAAWidth = 0;
        screenWidth = (float)(floor(screenWidth) + 1);
    }

    if (hinted)
    {
        offset = (screenWidth < 1.26f || (int(ceilf(screenWidth-0.25f)) & 1)) ? 0.5f : 0.0f;
    }
    else
    {
        float StrokeOffset = 0.25; // TO DO: Move to ToleranceParam
        offset = StrokeOffset;
    }

    // TO DO: Add check for blend mode, color matrix, etc.
    bool useStrokerAA = ((strokeStyle.Color >> 24) >= 240) && (strokeStyle.pFill.GetPtr() == 0);
    if (meshGenFlags & Mesh_Mask) 
        useStrokerAA = false;

#ifdef SF_RENDER_ENABLE_HAIRLINER
    if (screenWidth < 1.25f && !useStrokerAA && edgeAAWidth > 0)
    {
        gen->mHairliner.SetWidth((screenWidth < 1) ? edgeAAWidth : screenWidth-1 + edgeAAWidth);
        gen->mHairliner.SetToleranceParam(param);
        addStroke(gen, &gen->mHairliner, param, tr, startPos, strokeStyleIdx, offset, morphRatio);
        gen->mHairliner.Tessellate();
        ret = acquireTessMeshes(&gen->mHairliner, m2, pout, drawLayerIdx, strokeStyleIdx,
                                meshGenFlags, morphRatio);
    }
    else
#endif
    {
        StrokerTypes::LineJoinType join     = StrokerTypes::RoundJoin;
        StrokerTypes::LineCapType  startCap = StrokerTypes::RoundCap;
        StrokerTypes::LineCapType  endCap   = StrokerTypes::RoundCap;

        switch(strokeStyle.Flags & StrokeJoin_Mask)
        {
        case StrokeJoin_Round: break;
        case StrokeJoin_Bevel: join = StrokerTypes::BevelJoin; break;
        case StrokeJoin_Miter: join = StrokerTypes::MiterJoin; break;
        }

        switch(strokeStyle.Flags & StrokeCap_Mask)
        {
        case StrokeCap_Round:  break;
        case StrokeCap_None:   startCap = StrokerTypes::ButtCap;   break;
        case StrokeCap_Square: startCap = StrokerTypes::SquareCap; break;
        }

        switch(strokeStyle.Flags & StrokeEndCap_Mask)
        {
        case StrokeEndCap_Round:  break;
        case StrokeEndCap_None:   endCap = StrokerTypes::ButtCap;   break;
        case StrokeEndCap_Square: endCap = StrokerTypes::SquareCap; break;
        }

        screenWidth -= 2*edgeAAWidth;
        if (screenWidth < 0.1f)
        {
            if (useStrokerAA)
            {
                screenWidth  = 0;
            }
            else
            {
                edgeAAWidth -= 0.05f; 
                screenWidth  = 0.1f;
            }
        }

        if (useStrokerAA)
        {
            gen->mStrokerAA.SetSolidWidth(screenWidth);
            gen->mStrokerAA.SetEdgeAAWidth(edgeAAWidth * param.EdgeAAScale);
            gen->mStrokerAA.SetLineJoin(join);
            gen->mStrokerAA.SetStartLineCap(startCap);
            gen->mStrokerAA.SetEndLineCap(endCap);
            gen->mStrokerAA.SetMiterLimit(strokeStyle.Miter);
            gen->mStrokerAA.SetToleranceParam(param);
            addStroke(gen, &gen->mStrokerAA, param, tr, startPos, strokeStyleIdx, offset, morphRatio);
            ret = acquireTessMeshes(&gen->mStrokerAA, m2, pout, drawLayerIdx, strokeStyleIdx,
                                    meshGenFlags, morphRatio);
        }
        else
        {
            gen->mStroker.SetWidth(screenWidth);
            gen->mStroker.SetLineJoin(join);
            gen->mStroker.SetStartLineCap(startCap);
            gen->mStroker.SetEndLineCap(endCap);
            gen->mStroker.SetMiterLimit(strokeStyle.Miter);
            gen->mStroker.SetToleranceParam(param);
            gen->mTess.SetFillRule(Tessellator::FillStroker);
#ifdef SF_RENDER_ENABLE_EDGEAA
            gen->mTess.SetEdgeAAWidth(edgeAAWidth * param.EdgeAAScale);
#endif
            addStroke(gen, param, tr, startPos, strokeStyleIdx, offset, morphRatio);
            gen->mTess.Tessellate();
            ret = acquireTessMeshes(&gen->mTess, m2, pout, drawLayerIdx, strokeStyleIdx,
                                    meshGenFlags, morphRatio);
        }
    }
    gen->Clear();
//printf("-"); // DBG
    return ret;
}



//------------------------------------------------------------------------
RectF ShapeMeshProvider::getLayerBounds(unsigned drawLayer) const
{
    ShapePosInfo pos(GetLayerStartPos(drawLayer));
    ShapePathType pathType;
    float coord[Edge_MaxCoord];
    unsigned styles[3];
    bool first = true;
    RectF bounds(1e30f, 1e30f, -1e30f, -1e30f);
    Matrix2F im;
    while((pathType = pShapeData->ReadPathInfo(&pos, coord, styles)) != Shape_EndShape)
    {
        if (!first && pathType == Shape_NewLayer)
            break;
        first = false;

        if (styles[0] || styles[1])
        {
            ExpandBoundsToPath(*pShapeData, im, &pos, coord, &bounds);
        }
        else
        {
            pShapeData->SkipPathData(&pos);
        }
    }
    return bounds;
}


//------------------------------------------------------------------------
bool ShapeMeshProvider::generateImage9Grid(Scale9GridInfo* s9g, MeshBase *mesh, VertexOutput* verOut, unsigned drawLayer)
{
    I9gType i9g = DrawLayers[drawLayer].Image9GridType;
    RectF bounds = (i9g == I9gMergedSlice) ? IdentityBounds : getLayerBounds(drawLayer);


    const DrawLayerType& dl = DrawLayers[drawLayer];
    ShapePosInfo posInfo(dl.StartPos);
    ShapePathType pathType;
    float coords[Edge_MaxCoord];
    unsigned styles[3];
    pathType = pShapeData->ReadPathInfo(&posInfo, coords, styles);
    SF_ASSERT((styles[0] == 0) != (styles[1] == 0));

    unsigned styleIdx = styles[0] ? styles[0] : styles[1];
    FillStyleType fill;
    pShapeData->GetFillStyle(styleIdx, &fill);
    SF_ASSERT(fill.pFill.GetPtr() && fill.pFill->pImage.GetPtr());

    // Get coordinate->UV transformation matrix, which does divide by image size,
    // scaling and sub-image adjustment.
    Matrix2F uvGenMatrix(Matrix2F::NoInit);
    TextureManager* manager = mesh->GetRenderer()->GetHAL()->GetTextureManager();
    fill.pFill->pImage->GetUVNormMatrix(&uvGenMatrix, manager);

    Scale9GridTess tess(Memory::GetGlobalHeap(), *s9g, bounds, uvGenMatrix, fill.pFill->ImageMatrix);
    if (tess.GetIndexCount())
    {
        VertexOutput::Fill vfill = { tess.GetVertexCount(), tess.GetIndexCount(), &Image9GridVertex::Format, 0, 0, 0 };

        if (!verOut->BeginOutput(&vfill, 1, Matrix2F()))
            return false;

        verOut->SetVertices(0, 0, tess.GetVertices(), tess.GetVertexCount());
        verOut->SetIndices(0, 0, tess.GetIndices(), tess.GetIndexCount());

        verOut->EndOutput();
    }
    else
    {
        Image9GridVertex ver = { 0,0,0,0 };
        UInt16 tri[3];
        tri[0] = tri[1] = tri[2] = 0;
        VertexOutput::Fill vfill = { 1, 3, &Image9GridVertex::Format, 0, 0, 0 };

        if (!verOut->BeginOutput(&vfill, 1))
            return false; 

        verOut->SetVertices(0, 0, &ver, 1);
        verOut->SetIndices(0, 0, tri, 3);
        verOut->EndOutput();
    }
    return true;
}

//------------------------------------------------------------------------
bool ShapeMeshProvider::GetData(MeshBase *mesh, VertexOutput* verOut, unsigned meshGenFlags)
{
    Ptr<Scale9GridInfo> s9g;
    unsigned drawLayer = mesh->GetLayer();
    I9gType i9g = DrawLayers[drawLayer].Image9GridType;

    if (mesh->GetScale9Grid() && i9g != I9gNone)
    {
        s9g = *SF_HEAP_NEW(Memory::GetGlobalHeap()) Scale9GridInfo(mesh->GetScale9Grid(), Matrix2F());
        return generateImage9Grid(s9g, mesh, verOut, drawLayer);
    }

    if (mesh->GetScale9Grid())
        s9g = *SF_HEAP_NEW(Memory::GetGlobalHeap()) Scale9GridInfo(mesh->GetScale9Grid(), mesh->GetScale9Grid()->ViewMtx);

    unsigned strokeStyleIdx = GetLayerStroke(drawLayer);

    if (strokeStyleIdx == 0)
    {
        // Fill
        return tessellateFill(s9g, drawLayer, mesh, verOut, meshGenFlags);
    }

    // Stroke
    return tessellateStroke(s9g, strokeStyleIdx, drawLayer, mesh, verOut, meshGenFlags);
}


//------------------------------------------------------------------------
ComplexFill* ShapeMeshProvider::getComplexFill(unsigned drawLayer, unsigned fillIndex, 
                                               unsigned* imgFillStyle) const
{
    unsigned strokeStyleIdx = GetLayerStroke(drawLayer);
    SF_ASSERT(strokeStyleIdx != ~0U);

    ComplexFill* fill = 0;

    if (strokeStyleIdx == 0)
    {
        unsigned fillStyleIdx = GetStyleByFillIdx(drawLayer, fillIndex);
        if (fillStyleIdx)
        {
            // fillStyleIdx > 0 means complex fill
            FillStyleType fillStyle;
            pShapeData->GetFillStyle(fillStyleIdx, &fillStyle);
            fill = fillStyle.pFill.GetPtr();
            if (imgFillStyle)
               *imgFillStyle = fillStyleIdx;
            SF_ASSERT(fill); // If not solid, there must be an image/gradient
        }
    }
    else
    {
        StrokeStyleType stroke;
        pShapeData->GetStrokeStyle(strokeStyleIdx, &stroke);
        fill = stroke.pFill;
    }
    return fill;
}


//------------------------------------------------------------------------
Matrix2F ShapeMeshProvider::getMorphMatrix(unsigned drawLayer, unsigned fillIndex) const
{
    unsigned strokeStyleIdx = GetLayerStroke(drawLayer);
    SF_ASSERT(strokeStyleIdx != ~0U);
    SF_ASSERT(pMorphData.GetPtr());
    if (strokeStyleIdx == 0)
    {
        unsigned fillStyleIdx = GetStyleByFillIdx(drawLayer, fillIndex);
        SF_ASSERT(fillStyleIdx);
        // fillStyleIdx > 0 means complex fill
        FillStyleType fillStyle;
        pMorphData->pMorphTo->GetFillStyle(fillStyleIdx, &fillStyle);
        SF_ASSERT(fillStyle.pFill.GetPtr());
        return fillStyle.pFill->ImageMatrix;
    }
    StrokeStyleType stroke;
    pMorphData->pMorphTo->GetStrokeStyle(strokeStyleIdx, &stroke);
    if (stroke.pFill.GetPtr())
        return stroke.pFill->ImageMatrix;
    return Matrix2F();
}


//------------------------------------------------------------------------
unsigned ShapeMeshProvider::GetFillCount(unsigned drawLayer, unsigned meshGenFlags) const
{
    SF_UNUSED(meshGenFlags);
    return DrawLayers[drawLayer].FillCount;
}


//------------------------------------------------------------------------
void ShapeMeshProvider::GetFillData(FillData* pdata, unsigned drawLayer,
                                    unsigned fillIndex, unsigned meshGenFlags)
{
    if (meshGenFlags & Mesh_Mask)
    {
        *pdata = FillData(Fill_Mask);
        pdata->pVFormat = &VertexXY16iCF32::Format;
        return;
    }

    ComplexFill* fill = getComplexFill(drawLayer, fillIndex, 0);
    if (fill)
    {
        if (fill->pGradient.GetPtr())
        {
           *pdata = FillData(fill->pGradient);
        }
        else
        {
            SF_ASSERT(fill->pImage.GetPtr());
            *pdata = FillData(fill->pImage->GetAsImage(), fill->FillMode);
            if (DrawLayers[drawLayer].Image9GridType != I9gNone && (meshGenFlags & Mesh_Scale9))
            {
                pdata->PrimFill = PrimFill_UVTexture;
                pdata->pVFormat = &Image9GridVertex::Format;
            }
        }
    }
    else
    {
        *pdata = FillData();
    }
}


//------------------------------------------------------------------------
void ShapeMeshProvider::computeImgAdjustMatrix(const Scale9GridData* s9g, 
                                               unsigned drawLayer, unsigned imgFillStyle, 
                                               Matrix2F* mtx)
{
    ShapePosInfo pos(GetLayerStartPos(drawLayer));
    ShapePathType pathType;
    float coord[Edge_MaxCoord];
    unsigned styles[3];
    bool first = true;
    RectF bounds(1e30f, 1e30f, -1e30f, -1e30f);
    Matrix2F im;
    while((pathType = pShapeData->ReadPathInfo(&pos, coord, styles)) != Shape_EndShape)
    {
        if (!first && pathType == Shape_NewLayer)
            break;
        first = false;

        if (styles[0] == imgFillStyle || styles[1] == imgFillStyle)
        {
            ExpandBoundsToPath(*pShapeData, im, &pos, coord, &bounds);
        }
        else
        {
            pShapeData->SkipPathData(&pos);
        }
    }

    mtx->SetIdentity();
    if (bounds.x1 < bounds.x2 && bounds.y1 < bounds.y2)
    {
        // TO DO: Can be potentially optimized and cached in MeshBase
        Scale9GridInfo s(s9g, Matrix2F());
        float p[6] = { bounds.x1, bounds.y1, bounds.x2, bounds.y1, bounds.x2, bounds.y2 };
        s.Transform(&p[0], &p[1]);
        s.Transform(&p[2], &p[3]);
        s.Transform(&p[4], &p[5]);
        mtx->SetRectToParl(bounds.x1, bounds.y1, bounds.x2, bounds.y2, p);
    }
}

//------------------------------------------------------------------------
void ShapeMeshProvider::GetFillMatrix(MeshBase *mesh, Matrix2F* matrix, unsigned drawLayer,
                                      unsigned fillIndex, unsigned meshGenFlags)
{
//printf("M"); // DBG
    SF_UNUSED(meshGenFlags);
    unsigned imgFillStyle = 0;
    ComplexFill* fill = getComplexFill(drawLayer, fillIndex, &imgFillStyle);
    if (fill)
    {
        *matrix = fill->ImageMatrix;
        if (pMorphData.GetPtr() && mesh->GetMorphRatio() != 0)
        {
            // In case of morphing there must be direct matrices, not inverse ones.
            // TO DO: possibly revise, maybe it's better to keep direct matrices.
            matrix->SetLerp(fill->ImageMatrix.GetInverse(), 
                            getMorphMatrix(drawLayer, fillIndex).GetInverse(), 
                            mesh->GetMorphRatio());
            matrix->Invert();
        }

        Scale9GridData* s9g = 0;
        if (mesh) 
            s9g = mesh->GetScale9Grid();

        if (s9g)
        {
            Matrix2F mtx;
            computeImgAdjustMatrix(s9g, drawLayer, imgFillStyle, &mtx);
            matrix->Prepend(mtx.GetInverse());
        }

        if (fill->pImage)
        {
            // Get coordinate->UV transformation matrix, which does divide by image size,
            // scaling and sub-image adjustment.
            Matrix2F        uvGenMatrix(Matrix2F::NoInit);
            TextureManager* manager = mesh->GetRenderer()->GetHAL()->GetTextureManager();
            fill->pImage->GetUVGenMatrix(&uvGenMatrix, manager);
            matrix->Append(uvGenMatrix);
        }
    }
    else
    {
        *matrix = Matrix2F();
    }
}


#if defined(SF_AMP_SERVER)
void ShapeMeshProvider::clearStrokeCount()
{
    UInt32 numStrokes = 0;
    for (UPInt i = 0; i < DrawLayers.GetSize(); ++i)
    {
        if (DrawLayers[i].StrokeStyle != 0)
        {
            ++numStrokes;
        }
    }
    AmpServer::GetInstance().RemoveStrokes(numStrokes);
}
#endif


//------------------------------------------------------------------------
RectF ShapeMeshProvider::GetBounds(const Matrix2F& m) const
{
    // TO DO: Consider correct computation of stroke bounds that requires 
    // StrokeGenerator and ToleranceParam.
    if (pMorphData)
    {
        RectF b1 = ComputeBoundsFillAndStrokesSimplified(pMorphData->ShapeData1, m, Bound_OuterEdges);
        RectF b2 = ComputeBoundsFillAndStrokesSimplified(pMorphData->ShapeData2, m, Bound_OuterEdges);
        return b1.Union(b2);
   }
    return ComputeBoundsFillAndStrokesSimplified(*pShapeData, m, Bound_OuterEdges); 
}




//------------------------------------------------------------------------
RectF ShapeMeshProvider::GetCorrectBounds(const Matrix2F& m, float morphRatio, 
                                          StrokeGenerator* gen, const ToleranceParams* tol) const
{
    ShapePosInfo pos2(pShapeData->GetStartingPos());
    MorphInterpolator shape(pShapeData, pMorphData, morphRatio, pos2);
    if (gen)
    {
        SF_ASSERT(tol != 0);
        return ComputeBoundsFillAndStrokes(shape, m, gen, *tol, Bound_OuterEdges);
    }
    return ComputeBoundsFillAndStrokesSimplified(shape, m, Bound_OuterEdges);
}


//------------------------------------------------------------------------
bool ShapeMeshProvider::HitTestShape(const Matrix2F& m, float x, float y, float morphRatio,
                                     StrokeGenerator* gen, const ToleranceParams* tol,
                                     Scale9GridInfo* s9g) const
{
    ShapePosInfo pos2(pShapeData->GetStartingPos());
    MorphInterpolator shape(pShapeData, pMorphData, morphRatio, pos2);

    TransformerBase* tr = 0;
    TransformerWrapper<Matrix2F> trAffine;
    TransformerWrapper<Scale9GridInfo> trScale9;

    if (s9g)
    {
        trScale9 = TransformerWrapper<Scale9GridInfo>(s9g);
        tr = &trScale9;
    }
    else
    {
        trAffine = TransformerWrapper<Matrix2F>(&m);
        tr = &trAffine;
    }

    if (gen)
    {
        SF_ASSERT(tol != 0);
        return HitTestFillAndStrokes(shape, *tr, x, y, gen, *tol);
    }
    return HitTestFill(shape, m, x, y);
}



}} // Scaleform::Render
