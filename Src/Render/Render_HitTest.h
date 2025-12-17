/**************************************************************************

Filename    :   Render_HitTest.h
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef SF_Render_HitTest_H
#define SF_Render_HitTest_H

#include "Render_Math2D.h"
#include "Render_ShapeDataDefs.h"
#include "Render_TessGen.h"
#include "Render_TessCurves.h"


namespace Scaleform { namespace Render {

//==============================================================================
//
//  Hit-test functions. Publicly usable functions are:
//
//  The main function to test "point-in-shape" for both, fill and strokes.
//  ----------------------------------------------------------
//  bool HitTestFillAndStrokes(const ShapeDataInterface& shape, 
//                             const Transformer& trans, float x, float y, 
//                             StrokeGenerator* gen, const ToleranceParams& tol);
//
//  Hit-test for fill only. The function does not require curve subdivision.
//  ----------------------------------------------------------
//  bool HitTestFill(const ShapeDataInterface& shape, const Transformer& trans, float x, float y);
//
//==============================================================================


namespace Math2D
{

//--------------------------------------------------------------------
struct QuadCoordType
{
    float x1, y1, x2, y2, x3, y3;
};

//--------------------------------------------------------------------
template<class CurveType>
void SubdivideQuadCurve(float x1, float y1, 
                        float x2, float y2, 
                        float x3, float y3, 
                        float t, 
                        CurveType* c1, CurveType* c2)
{
    float x12  = x1  + t*(x2  - x1);
    float y12  = y1  + t*(y2  - y1);
    float x23  = x2  + t*(x3  - x2);
    float y23  = y2  + t*(y3  - y2);
    float x123 = x12 + t*(x23 - x12);
    float y123 = y12 + t*(y23 - y12);
    c1->x1 = x1; 
    c1->y1 = y1;
    c1->x2 = x12; 
    c1->y2 = y12; 
    c1->x3 = x123;
    c1->y3 = y123;
    c2->x1 = x123;
    c2->y1 = y123; 
    c2->x2 = x23; 
    c2->y2 = y23;
    c2->x3 = x3;
    c2->y3 = y3;
}

//--------------------------------------------------------------------
inline float CalcPointOnQuadCurve1D(float x1, float x2, float x3, float t)
{
    x1 += t*(x2 - x1);
    x2 += t*(x3 - x2);
    return x1 + t*(x2 - x1);
}

int CheckQuadraticIntersection(int styleCount, float x1, float y1, float x2, float y2, 
                               float x3, float y3, float x, float y);


int CheckCubicIntersection(int styleCount, float x1, float y1, float x2, float y2, 
                           float x3, float y3, float x4, float y4, float x, float y);
} // Math2D



// HitTest for shape fill only. Does not require any memory allocations, render gen, etc.
//--------------------------------------------------------------------
template<class Transformer> 
bool HitTestFill(const ShapeDataInterface& shape, const Transformer& trans, float x, float y)
{
    ShapePosInfo pos(shape.GetStartingPos());
    ShapePathType pathType;
    float coord[Edge_MaxCoord];
    unsigned styles[3];
    int styleCount = 0;

    while((pathType = shape.ReadPathInfo(&pos, coord, styles)) != Shape_EndShape)
    {
        if (pathType == Shape_NewLayer)
        {
            if (styleCount)
                return true;
            styleCount = 0;
        }

        // For compound shapes we only need to consider the "outer" paths,
        // that is, the ones that have one "zero fill" on one side.
        //------------------------
        if ((styles[0] == 0) != (styles[1] == 0))
        {
            trans.Transform(&coord[0], &coord[1]);
            float lastX = coord[0];
            float lastY = coord[1];
            float x1, y1, x2, y2;
            PathEdgeType edgeType;
            while((edgeType = shape.ReadEdge(&pos, coord)) != Edge_EndPath)
            {
                if(edgeType == Edge_LineTo)
                {
                    trans.Transform(&coord[0], &coord[1]);
                    x1 = lastX; 
                    y1 = lastY;
                    x2 = coord[0]; 
                    y2 = coord[1];
                    if(y1 > y2)
                    {
                        Alg::Swap(x1, x2);
                        Alg::Swap(y1, y2);
                    }
                    if (y >= y1 && y < y2 && Math2D::CrossProduct(x1, y1, x2, y2, x, y) > 0)
                    {
                        styleCount ^= 1;
                    }
                    lastX = coord[0];
                    lastY = coord[1];
                }
                else
                if(edgeType == Edge_QuadTo)
                {
                    trans.Transform(&coord[0], &coord[1]);
                    trans.Transform(&coord[2], &coord[3]);
                    styleCount = Math2D::CheckQuadraticIntersection(styleCount, lastX, lastY, coord[0], coord[1], coord[2], coord[3], x, y);
                    lastX = coord[2];
                    lastY = coord[3];
                }
                else
                if(edgeType == Edge_CubicTo)
                {
                    trans.Transform(&coord[0], &coord[1]);
                    trans.Transform(&coord[2], &coord[3]);
                    trans.Transform(&coord[4], &coord[5]);
                    styleCount = Math2D::CheckCubicIntersection(styleCount, lastX, lastY, coord[0], coord[1], coord[2], coord[3], coord[4], coord[5], x, y);
                    lastX = coord[4];
                    lastY = coord[5];
                }
            }
        }
        else
        {
            shape.SkipPathData(&pos);
        }
    }
    return styleCount != 0;
}



//--------------------------------------------------------------------
template<class Transformer> 
ShapePathType AddStrokeToSorter(const ShapeDataInterface& shape, ShapePosInfo* pos, unsigned strokeStyleIdx, 
                                const Transformer& trans, StrokeGenerator* gen)
{
    ShapePathType pathType;
    ShapePosInfo prevPos;
    float coord[Edge_MaxCoord];
    unsigned styles[3];
    bool first = true;

    gen->mStrokeSorter.Clear();
    for(;;)
    {
        prevPos = *pos;
        pathType = shape.ReadPathInfo(pos, coord, styles);

        if (pathType == Shape_EndShape)
        {
            *pos = prevPos;
            break;
        }

        if (pathType == Shape_NewLayer && !first)
        {
            *pos = prevPos;
            break;
        }

        if (styles[2] == strokeStyleIdx)
        {
            trans.Transform(&coord[0], &coord[1]);
            gen->mStrokeSorter.AddVertexNV(coord[0], coord[1]);
            PathEdgeType edgeType;
            while((edgeType = shape.ReadEdge(pos, coord)) != Edge_EndPath)
            {
                if(edgeType == Edge_LineTo)
                {
                    trans.Transform(&coord[0], &coord[1]);
                    gen->mStrokeSorter.AddVertexNV(coord[0], coord[1]);
                }
                else
                if(edgeType == Edge_QuadTo)
                {
                    trans.Transform(&coord[0], &coord[1]);
                    trans.Transform(&coord[2], &coord[3]);
                    gen->mStrokeSorter.AddQuad(coord[0], coord[1], coord[2], coord[3]);
                }
                else
                if(edgeType == Edge_CubicTo)
                {
                    trans.Transform(&coord[0], &coord[1]);
                    trans.Transform(&coord[2], &coord[3]);
                    trans.Transform(&coord[4], &coord[5]);
                    gen->mStrokeSorter.AddCubic(coord[0], coord[1], coord[2], coord[3], coord[4], coord[5]);
                }
            }
            gen->mStrokeSorter.FinalizePath(0, 0, false, false);
        }
        else
        {
            shape.SkipPathData(pos);
        }
        first = false;
    }
    gen->mStrokeSorter.Sort();
    return pathType;
}


//--------------------------------------------------------------------
bool HitTestRoundStroke(const VertexPath& path, float w, float x, float y);
bool HitTestGeneralStroke(const VertexPath& path, float x, float y);



//--------------------------------------------------------------------
template<class Transformer> 
bool HitTestRoundJoinStroke(const ShapeDataInterface& shape, ShapePosInfo* pos, unsigned strokeStyleIdx, 
                            const Transformer& trans, float scaledWidth, float x, float y,
                            StrokeGenerator* gen, const ToleranceParams& tol)
{
    AddStrokeToSorter(shape, pos, strokeStyleIdx, trans, gen);

    unsigned i, j;
    for (i = 0; i < gen->mStrokeSorter.GetPathCount(); ++i)
    {
        unsigned n = gen->mStrokeSorter.GetVertexCount(i);
        gen->mPath.Clear();
        for (j = 0; j < n; )
        {
            const StrokeSorter::VertexType& v1 = gen->mStrokeSorter.GetVertex(i, j++);
            if (v1.segType == Math2D::Seg_MoveTo || v1.segType == Math2D::Seg_LineTo)
            {
                gen->mPath.AddVertex(v1.x, v1.y);
            }
            else
            if (v1.segType == Math2D::Seg_QuadTo)
            {
                const StrokeSorter::VertexType& v2 = gen->mStrokeSorter.GetVertex(i, j++);
                TessellateQuadCurve(&gen->mPath, tol, v1.x, v1.y, v2.x, v2.y);
            }
            if (v1.segType == Math2D::Seg_CubicTo)
            {
                const StrokeSorter::VertexType& v2 = gen->mStrokeSorter.GetVertex(i, j++);
                const StrokeSorter::VertexType& v3 = gen->mStrokeSorter.GetVertex(i, j++);
                TessellateCubicCurve(&gen->mPath, tol, v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
            }
        }
        if (HitTestRoundStroke(gen->mPath, scaledWidth, x, y))
            return true;
    }
    return false;
}



// HitTest a single stroke in a single layer.
//--------------------------------------------------------------------
template<class Transformer> 
bool HitTestSingleStroke(const ShapeDataInterface& shape, ShapePosInfo* pos, unsigned strokeStyleIdx, 
                         const Transformer& trans, float x, float y, 
                         StrokeGenerator* gen, const ToleranceParams& tol)
{
    StrokeStyleType strokeStyle;
    shape.GetStrokeStyle(strokeStyleIdx, &strokeStyle);

    float widthScale = 1.0f;
    switch(strokeStyle.Flags & StrokeScaling_Mask)
    {
    case StrokeScaling_Normal:      widthScale = trans.GetScale();  break;
    case StrokeScaling_Horizontal:  widthScale = trans.GetXScale(); break;
    case StrokeScaling_Vertical:    widthScale = trans.GetYScale(); break;
    case StrokeScaling_None:                                        break;
    }

    float scaledWidth = strokeStyle.Width * widthScale;

    if ((strokeStyle.Flags & StrokeJoin_Mask) == StrokeJoin_Round &&
        (strokeStyle.Flags & StrokeCap_Mask)  == StrokeCap_Round)
    {
        return HitTestRoundJoinStroke(shape, pos, strokeStyleIdx, trans, scaledWidth, x, y, gen, tol);
    }

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

    gen->mStroker.SetWidth(scaledWidth);
    gen->mStroker.SetLineJoin(join);
    gen->mStroker.SetStartLineCap(startCap);
    gen->mStroker.SetEndLineCap(endCap);
    gen->mStroker.SetMiterLimit(strokeStyle.Miter);
    gen->mStroker.SetToleranceParam(tol);

    AddStrokeToSorter(shape, pos, strokeStyleIdx, trans, gen);

    unsigned i, j;
    gen->mStroker.Clear();

    for (i = 0; i < gen->mStrokeSorter.GetPathCount(); ++i)
    {
        gen->mPath.Clear();
        unsigned n = gen->mStrokeSorter.GetVertexCount(i);
        for (j = 0; j < n; )
        {
            const StrokeSorter::VertexType& v1 = gen->mStrokeSorter.GetVertex(i, j++);
            if (v1.segType == Math2D::Seg_MoveTo || v1.segType == Math2D::Seg_LineTo)
            {
                gen->mStroker.AddVertex(v1.x, v1.y);
            }
            else
            if (v1.segType == Math2D::Seg_QuadTo)
            {
                const StrokeSorter::VertexType& v2 = gen->mStrokeSorter.GetVertex(i, j++);
                TessellateQuadCurve(&gen->mStroker, tol, v1.x, v1.y, v2.x, v2.y);
            }
            else
            if (v1.segType == Math2D::Seg_CubicTo)
            {
                const StrokeSorter::VertexType& v2 = gen->mStrokeSorter.GetVertex(i, j++);
                const StrokeSorter::VertexType& v3 = gen->mStrokeSorter.GetVertex(i, j++);
                TessellateCubicCurve(&gen->mStroker, tol, v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
            }
        }
        gen->mStroker.GenerateStroke(&gen->mPath);
        if (HitTestGeneralStroke(gen->mPath, x, y))
            return true;
    }
    return false;
}


// HitTest all strokes in a single layer.
//--------------------------------------------------------------------
template<class Transformer> 
bool HitTestLayerStrokes(const ShapeDataInterface& shape, ShapePosInfo* pos,
                         const Transformer& trans, float x, float y, 
                         StrokeGenerator* gen, const ToleranceParams& tol)
{
    BitSet strokeStyles(gen->Heap1.GetMemoryHeap());

    ShapePathType pathType;
    ShapePosInfo prevPos;
    float coord[Edge_MaxCoord];
    unsigned styles[3];
    bool first = true;

    for(;;)
    {
        prevPos = *pos;
        pathType = shape.ReadPathInfo(pos, coord, styles);

        if (pathType == Shape_EndShape)
        {
            *pos = prevPos;
            break;
        }

        if (pathType == Shape_NewLayer && !first)
        {
            *pos = prevPos;
            break;
        }

        if (styles[2] && strokeStyles[styles[2]] == 0)
        {
            if (HitTestSingleStroke(shape, &prevPos, styles[2], trans, x, y, gen, tol))
                return true;
            strokeStyles.Set(styles[2]);
        }
        shape.SkipPathData(pos);
        first = false;
    }
    return false;
}



// HitTest all strokes in a shape.
//--------------------------------------------------------------------
template<class Transformer> 
bool HitTestShapeStrokes(const ShapeDataInterface& shape, 
                         const Transformer& trans, float x, float y, 
                         StrokeGenerator* gen, const ToleranceParams& tol)
{
    ShapePathType pathType;
    ShapePosInfo pos(shape.GetStartingPos());
    ShapePosInfo prevPos;
    float coord[Edge_MaxCoord];
    unsigned styles[3];
    bool first = true;

    for(;;)
    {
        prevPos = pos;
        pathType = shape.ReadPathInfo(&pos, coord, styles);
        if (pathType == Shape_EndShape)
            break;

        SF_ASSERT(pathType == Shape_NewLayer || first);

        pos = prevPos;
        if (HitTestLayerStrokes(shape, &pos, trans, x, y, gen, tol))
            return true;

        first = false;
    }
    return false;
}


// HitTest all fills and strokes in a shape.
//--------------------------------------------------------------------
template<class Transformer> 
bool HitTestFillAndStrokes(const ShapeDataInterface& shape, 
                           const Transformer& trans, float x, float y, 
                           StrokeGenerator* gen, const ToleranceParams& tol)
{
    // To test the fill layers there's no need to have actual fill styles, since
    // the shape may have no styles at all (font glyphs). So that, it's mandatory
    // to check the hit test of fills anyways.
    //--------------------------
    if (HitTestFill(shape, trans, x, y))
    {
        return true;
    }

    // The stroke cannot exist without the stroke styles, thus it's possible 
    // to optimize it and check only if there are actual stroke styles.
    //--------------------------
    if (shape.GetStrokeStyleCount())
    {
        shape.Rewind();
        return HitTestShapeStrokes(shape, trans, x, y, gen, tol);
    }
    return false;
}



}} // Scaleform::Render

#endif
