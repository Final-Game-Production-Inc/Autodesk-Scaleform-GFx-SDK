/**************************************************************************

Filename    :   Render_BoundBox.h
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef SF_Render_BoundBox_H
#define SF_Render_BoundBox_H

#include "Render_Types2D.h"
#include "Render_HitTest.h"
#include "Render_TessDefs.h"

namespace Scaleform { namespace Render {



//==============================================================================
//
//  Bounding box functions. Publicly usable functions are:
//
//  The main function for correct bounding box computation. It considers
//  both, fill and strokes of the shape, but requires StrokeGenerator that 
//  provides  the stroker and all necessary containers, and ToleranceParams 
//  for curve subdivision. 
//  ----------------------------------------------------------
//  RectF ComputeBoundsFillAndStrokes(const ShapeDataInterface& shape, const Transformer& trans, 
//                                    StrokeGenerator* gen, const ToleranceParams& tol,
//                                    BoundEdges edgesToCheck=Bound_OuterEdges);
//
//
//  Simplified bounding box computation. The function assumes that all strokes
//  have round joins and round caps, but it doesn't require any containers, nor
//  ToleranceParams. It may result in inaccurate (smaller) bounding boxes in case
//  the stroke has square caps and miter joins.
//  ----------------------------------------------------------
//  RectF ComputeBoundsFillAndStrokesSimplified(const ShapeDataInterface& shape, const Transformer& trans, 
//                                              BoundEdges edgesToCheck=Bound_OuterEdges)
//
//
//  Additional function that can be publically used. It computes the bounding box of fill only.
//  Can be used to compute bounds of vector glyphs, or any other shapes that don't have styles.
//  ----------------------------------------------------------
//  RectF ComputeBoundsFill(const ShapeDataInterface& shape, const Transformer& trans,
//                          BoundEdges edgesToCheck=Bound_OuterEdges)
//
//==============================================================================



//--------------------------------------------------------------------
enum BoundEdges
{
    // For compound shapes we only need to consider the "outer" paths,
    // that is, the ones that have one "zero fill" on one side. This version
    // looks like this: if ((styles[0] == 0) != (styles[1] == 0))
    // Works faster.
    Bound_OuterEdges, 

    // More strict version looks like this: if (styles[0] != styles[1] == 0)
    // it considers all paths, including inner ones and computes correct 
    // result even for incorrect shapes, with "non-zero fill" outside.
    // Works slower
    Bound_FillEdges,

    // Consider all edges, including stroke ones. Used mostly for testing 
    // purposes.
    Bound_AllEdges
};



//--------------------------------------------------------------------
template<class Transformer> 
void ExpandBoundsToPath(const ShapeDataInterface& shape, const Transformer& trans, 
                        ShapePosInfo* pos, float* coord, RectF* bounds)
{
    trans.Transform(&coord[0], &coord[1]);
    bounds->ExpandToPoint(coord[0], coord[1]);
    float lastX = coord[0];
    float lastY = coord[1];

    PathEdgeType edgeType;
    while((edgeType = shape.ReadEdge(pos, coord)) != Edge_EndPath)
    {
        if(edgeType == Edge_LineTo)
        {
            trans.Transform(&coord[0], &coord[1]);
            bounds->ExpandToPoint(coord[0], coord[1]);
            lastX = coord[0];
            lastY = coord[1];
        }
        else
        if(edgeType == Edge_QuadTo)
        {
            trans.Transform(&coord[0], &coord[1]);
            trans.Transform(&coord[2], &coord[3]);

            float x, y, t;
            t = Math2D::QuadCurveExtremum(lastX, coord[0], coord[2]);
            if (t > 0 && t < 1)
            {
                Math2D::PointOnQuadCurve(lastX, lastY, 
                                         coord[0], coord[1], 
                                         coord[2], coord[3],
                                         t, &x, &y);
                bounds->ExpandToPoint(x, y);
            }
            t = Math2D::QuadCurveExtremum(lastY, coord[1], coord[3]);
            if (t > 0 && t < 1)
            {
                Math2D::PointOnQuadCurve(lastX, lastY, 
                                         coord[0], coord[1], 
                                         coord[2], coord[3],
                                         t, &x, &y);
                bounds->ExpandToPoint(x, y);
            }
            bounds->ExpandToPoint(coord[2], coord[3]);
            lastX = coord[2];
            lastY = coord[3];
        }
        else
        if(edgeType == Edge_CubicTo)
        {
            trans.Transform(&coord[0], &coord[1]);
            trans.Transform(&coord[2], &coord[3]);
            trans.Transform(&coord[3], &coord[4]);

            float x, y, t1, t2;

            Math2D::CubicCurveExtremum(lastX, coord[0], coord[2], coord[4], &t1, &t2);
            if (t1 > 0 && t1 < 1)
            {
                Math2D::PointOnCubicCurve(lastX, lastY, 
                                          coord[0], coord[1], 
                                          coord[2], coord[3],
                                          coord[4], coord[5],
                                          t1, &x, &y);
                bounds->ExpandToPoint(x, y);
            }
            if (t2 > 0 && t2 < 1)
            {
                Math2D::PointOnCubicCurve(lastX, lastY, 
                                          coord[0], coord[1], 
                                          coord[2], coord[3],
                                          coord[4], coord[5],
                                          t2, &x, &y);
                bounds->ExpandToPoint(x, y);
            }

            Math2D::CubicCurveExtremum(lastY, coord[1], coord[3], coord[5], &t1, &t2);
            if (t1 > 0 && t1 < 1)
            {
                Math2D::PointOnCubicCurve(lastX, lastY, 
                                          coord[0], coord[1], 
                                          coord[2], coord[3],
                                          coord[4], coord[5],
                                          t1, &x, &y);
                bounds->ExpandToPoint(x, y);
            }
            if (t2 > 0 && t2 < 1)
            {
                Math2D::PointOnCubicCurve(lastX, lastY, 
                                          coord[0], coord[1], 
                                          coord[2], coord[3],
                                          coord[4], coord[5],
                                          t2, &x, &y);
                bounds->ExpandToPoint(x, y);
            }

            bounds->ExpandToPoint(coord[4], coord[5]);
            lastX = coord[4];
            lastY = coord[5];
        }

    }
}


// Expand bounds to fill only. Does not require any memory allocations, render gen, etc.
// Optional layerBits defines which layers to test, if null, test all, otherwise skip 
// zero-bit ones. This feature may be useful to exclude fully transparent and or 
// invisible layers from computing
//--------------------------------------------------------------------
template<class Transformer> 
void ExpandBoundsToFill(const ShapeDataInterface& shape, const Transformer& trans, RectF* bounds,
                        BoundEdges edgesToCheck=Bound_OuterEdges)
{
    ShapePosInfo pos(shape.GetStartingPos());
    ShapePathType pathType;
    float coord[Edge_MaxCoord];
    unsigned styles[3];

    while((pathType = shape.ReadPathInfo(&pos, coord, styles)) != Shape_EndShape)
    {
        bool compute = false;
        switch(edgesToCheck)
        {
        case Bound_OuterEdges: compute = (styles[0] == 0) != (styles[1] == 0); break;
        case Bound_FillEdges:  compute = styles[0] != styles[1];               break;
        case Bound_AllEdges:   compute = true;                                 break;
        }
        //------------------------
        if (compute)
        {
            ExpandBoundsToPath(shape, trans, &pos, coord, bounds);
        }
        else
        {
            shape.SkipPathData(&pos);
        }
    }
}




// Compute bounds of shape fill.
//--------------------------------------------------------------------
template<class Transformer> 
RectF ComputeBoundsFill(const ShapeDataInterface& shape, const Transformer& trans,
                        BoundEdges edgesToCheck=Bound_OuterEdges)
{
    RectF bounds(1e30f, 1e30f, -1e30f, -1e30f);
    ExpandBoundsToFill(shape, trans, &bounds, edgesToCheck);
    return bounds;
}



//--------------------------------------------------------------------
template<class Transformer> 
RectF ComputeBoundsRoundStroke(const ShapeDataInterface& shape, const Transformer& trans, 
                               ShapePosInfo* pos, float* coord, unsigned* styles)
{
    RectF bounds(1e30f, 1e30f, -1e30f, -1e30f);

    StrokeStyleType strokeStyle;
    shape.GetStrokeStyle(styles[2], &strokeStyle);

    float widthScale = 1.0f;
    switch(strokeStyle.Flags & StrokeScaling_Mask)
    {
    case StrokeScaling_Normal:      widthScale = trans.GetScale();  break;
    case StrokeScaling_Horizontal:  widthScale = trans.GetXScale(); break;
    case StrokeScaling_Vertical:    widthScale = trans.GetYScale(); break;
    case StrokeScaling_None:                                        break;
    }

    float scaledWidth = strokeStyle.Width * widthScale;

    ExpandBoundsToPath(shape, trans, pos, coord, &bounds);

    if (bounds.IsNormal())
    {
        scaledWidth /= 2;
        bounds.x1 -= scaledWidth;
        bounds.y1 -= scaledWidth;
        bounds.x2 += scaledWidth;
        bounds.y2 += scaledWidth;
    }
    return bounds;
}



//--------------------------------------------------------------------
template<class Transformer> 
void ExpandBoundsToStrokesSimplified(const ShapeDataInterface& shape, const Transformer& trans, RectF* bounds)
{
    ShapePosInfo pos(shape.GetStartingPos());
    ShapePathType pathType;
    float coord[Edge_MaxCoord];
    unsigned styles[3];

    while((pathType = shape.ReadPathInfo(&pos, coord, styles)) != Shape_EndShape)
    {
        if (styles[2])
        {
            RectF b2 = ComputeBoundsRoundStroke(shape, trans, &pos, coord, styles);
            if (b2.IsNormal())
            {
                bounds->ExpandToPoint(b2.x1, b2.y1);
                bounds->ExpandToPoint(b2.x2, b2.y2);
            }
        }
        else
        {
            shape.SkipPathData(&pos);
        }
    }
}




// Compute bounds of fill and strokes in shape using a simplified mode. It doesn't require
// StrokeGenerator nor ToleranceParams, but it assumes all the strokes have round joins and caps.
//--------------------------------------------------------------------
template<class Transformer> 
RectF ComputeBoundsFillAndStrokesSimplified(const ShapeDataInterface& shape, const Transformer& trans, 
                                            BoundEdges edgesToCheck=Bound_OuterEdges)
{
    RectF bounds(1e30f, 1e30f, -1e30f, -1e30f);
    ExpandBoundsToFill(shape, trans, &bounds, edgesToCheck);
    if (shape.GetStrokeStyleCount())
    {
        shape.Rewind();
        ExpandBoundsToStrokesSimplified(shape, trans, &bounds);
    }
    return bounds;
}









// In case of round joins and caps it's more efficient to compute bounds
// of a path and then to expand it to the half of stroke width.
//--------------------------------------------------------------------
template<class Transformer> 
RectF ComputeBoundsRoundStrokeSorter(const ShapeDataInterface& shape, ShapePosInfo* pos, unsigned strokeStyleIdx, 
                                     const Transformer& trans, float scaledWidth, StrokeGenerator* gen)
{
    RectF bounds(1e30f, 1e30f, -1e30f, -1e30f);
    AddStrokeToSorter(shape, pos, strokeStyleIdx, trans, gen);

    unsigned i, j;
    for (i = 0; i < gen->mStrokeSorter.GetPathCount(); ++i)
    {
        unsigned n = gen->mStrokeSorter.GetVertexCount(i);
        float lastX = 0;
        float lastY = 0;
        for (j = 0; j < n; )
        {
            const StrokeSorter::VertexType& v1 = gen->mStrokeSorter.GetVertex(i, j++);

            if (v1.segType == Math2D::Seg_MoveTo || v1.segType == Math2D::Seg_LineTo)
            {
                bounds.ExpandToPoint(lastX = v1.x, lastY = v1.y);
            }
            else
            if (v1.segType == Math2D::Seg_QuadTo)
            {
                const StrokeSorter::VertexType& v2 = gen->mStrokeSorter.GetVertex(i, j++);
                float x, y, t;
                t = Math2D::QuadCurveExtremum(lastX, v1.x, v2.x);
                if (t > 0 && t < 1)
                {
                    Math2D::PointOnQuadCurve(lastX, lastY, v1.x, v1.y, v2.x, v2.y, t, &x, &y);
                    bounds.ExpandToPoint(x, y);
                }
                t = Math2D::QuadCurveExtremum(lastY, v1.y, v2.y);
                if (t > 0 && t < 1)
                {
                    Math2D::PointOnQuadCurve(lastX, lastY, v1.x, v1.y, v2.x, v2.y, t, &x, &y);
                    bounds.ExpandToPoint(x, y);
                }
                bounds.ExpandToPoint(v2.x, v2.y);
                lastX = v2.x;
                lastY = v2.y;
            }
            else
            if (v1.segType == Math2D::Seg_CubicTo)
            {
                const StrokeSorter::VertexType& v2 = gen->mStrokeSorter.GetVertex(i, j++);
                const StrokeSorter::VertexType& v3 = gen->mStrokeSorter.GetVertex(i, j++);

                float x, y, t1, t2;
                Math2D::CubicCurveExtremum(lastX, v1.x, v2.x, v3.x, &t1, &t2);
                if (t1 > 0 && t1 < 1)
                {
                    Math2D::PointOnCubicCurve(lastX, lastY, v1.x, v1.y, v2.x, v2.y, v3.x, v3.y, t1, &x, &y);
                    bounds.ExpandToPoint(x, y);
                }
                if (t2 > 0 && t2 < 1)
                {
                    Math2D::PointOnCubicCurve(lastX, lastY, v1.x, v1.y, v2.x, v2.y, v3.x, v3.y, t2, &x, &y);
                    bounds.ExpandToPoint(x, y);
                }

                Math2D::CubicCurveExtremum(lastY, v1.y, v2.y, v3.y, &t1, &t2);
                if (t1 > 0 && t1 < 1)
                {
                    Math2D::PointOnCubicCurve(lastX, lastY, v1.x, v1.y, v2.x, v2.y, v3.x, v3.y, t1, &x, &y);
                    bounds.ExpandToPoint(x, y);
                }
                if (t2 > 0 && t2 < 1)
                {
                    Math2D::PointOnCubicCurve(lastX, lastY, v1.x, v1.y, v2.x, v2.y, v3.x, v3.y, t2, &x, &y);
                    bounds.ExpandToPoint(x, y);
                }
                bounds.ExpandToPoint(v3.x, v3.y);
                lastX = v3.x;
                lastY = v3.y;
            }
        }
    }

    if (bounds.IsNormal())
    {
        scaledWidth /= 2;
        bounds.x1 -= scaledWidth;
        bounds.y1 -= scaledWidth;
        bounds.x2 += scaledWidth;
        bounds.y2 += scaledWidth;
    }
    return bounds;
}



// Compute bounds of a single stroke in a single layer.
//--------------------------------------------------------------------
template<class Transformer> 
RectF ComputeBoundsSingleStroke(const ShapeDataInterface& shape, ShapePosInfo* pos, unsigned strokeStyleIdx, 
                                const Transformer& trans, StrokeGenerator* gen, const ToleranceParams& tol)
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
        return ComputeBoundsRoundStrokeSorter(shape, pos, strokeStyleIdx, trans, scaledWidth, gen);
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

    RectF bounds(1e30f, 1e30f, -1e30f, -1e30f);
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
        for (j = 0; j < gen->mPath.GetNumVertices(); ++j)
        {
            const VertexPath::VertexType& v = gen->mPath.GetVertex(j);
            bounds.ExpandToPoint(v.x, v.y);
        }
    }
    return bounds;
}




// Expand bounds to all strokes in a single layer.
//--------------------------------------------------------------------
template<class Transformer> 
void ExpandBoundsToLayerStrokes(const ShapeDataInterface& shape, ShapePosInfo* pos,
                                const Transformer& trans, StrokeGenerator* gen, 
                                const ToleranceParams& tol, RectF* bounds)
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
            RectF b2 = ComputeBoundsSingleStroke(shape, &prevPos, styles[2], trans, gen, tol);
            if (b2.IsNormal())
            {
                bounds->ExpandToPoint(b2.x1, b2.y1);
                bounds->ExpandToPoint(b2.x2, b2.y2);
            }
            strokeStyles.Set(styles[2]);
        }
        shape.SkipPathData(pos);
        first = false;
    }
}



// Expand bounds to all strokes in a shape.
//--------------------------------------------------------------------
template<class Transformer> 
void ExpandBoundsToStrokes(const ShapeDataInterface& shape, const Transformer& trans, 
                           StrokeGenerator* gen, const ToleranceParams& tol, RectF* bounds)
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
        ExpandBoundsToLayerStrokes(shape, &pos,trans, gen, tol, bounds);
        first = false;
    }
}




// Compute bounds of fill and strokes in shape.
//--------------------------------------------------------------------
template<class Transformer> 
RectF ComputeBoundsFillAndStrokes(const ShapeDataInterface& shape, const Transformer& trans, 
                                  StrokeGenerator* gen, const ToleranceParams& tol,
                                  BoundEdges edgesToCheck=Bound_OuterEdges)
{
    RectF bounds(1e30f, 1e30f, -1e30f, -1e30f);
    ExpandBoundsToFill(shape, trans, &bounds, edgesToCheck);
    if (shape.GetStrokeStyleCount())
    {
        shape.Rewind();
        ExpandBoundsToStrokes(shape, trans, gen, tol, &bounds);
    }
    return bounds;
}






}} // Scaleform::Render

#endif
