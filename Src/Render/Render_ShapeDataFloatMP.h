/**************************************************************************

PublicHeader:   Render
Filename    :   Render_ShapeDataFloatMP.h
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef SF_Render_ShapeDataFloatMP_H
#define SF_Render_ShapeDataFloatMP_H

#include "Render_ShapeDataFloat.h"
#include "Render_ShapeMeshProvider.h"

namespace Scaleform { namespace Render {

// Convenience wrapper class that combines ShapeDataInterface and ShapeMeshProvider
//------------------------------------------------------------------------
class ShapeDataFloatMP : public ShapeMeshProvider
{
public:
    ShapeDataFloatMP();

    void Clear() { pData->Clear(); }
    unsigned AddFillStyle(const FillStyleType& fill) { return pData->AddFillStyle(fill); }
    unsigned AddStrokeStyle(const StrokeStyleType& str) { return pData->AddStrokeStyle(str); }

    unsigned AddStrokeStyle(float width, unsigned flags, float miter, UInt32 color)
    { 
        return pData->AddStrokeStyle(width, flags, miter, color);
    }

    void StartLayer() 
    { 
        pData->StartLayer(); 
    }

    void StartPath(unsigned leftStyle, unsigned rightStyle, unsigned strokeStyle) 
    { 
        pData->StartPath(leftStyle, rightStyle, strokeStyle); 
    }

    void MoveTo(float x, float y)
    {
        pData->MoveTo(x, y); 
    }

    void LineTo(float x, float y)
    {
        pData->LineTo(x, y); 
    }

    void QuadTo(float cx, float cy, float ax, float ay)
    {
        pData->QuadTo(cx, cy, ax, ay); 
    }

    void CubicTo(float cx1, float cy1, float cx2, float cy2, float ax, float ay)
    {
        pData->CubicTo(cx1, cy1, cx2, cy2, ax, ay); 
    }

    void RectanglePath(float x1, float y1, float x2, float y2)
    {
        pData->MoveTo(x1, y1);
        pData->LineTo(x2, y1);
        pData->LineTo(x2, y2);
        pData->LineTo(x1, y2);
        pData->ClosePath();
        pData->EndPath();
    }

    void ClosePath()
    {
        pData->ClosePath();
    }

    void EndPath()
    {
        pData->EndPath();
    }

    void EndShape()
    {
        pData->EndShape();
    }

    void CountLayers();

    unsigned GetFillStyleCount() const { return pData->GetFillStyleCount(); }
    unsigned GetStrokeStyleCount() const { return pData->GetStrokeStyleCount(); }
    void     GetFillStyle(unsigned idx, FillStyleType* p) const { return pData->GetFillStyle(idx, p); }
    void     GetStrokeStyle(unsigned idx, StrokeStyleType* p) const { return pData->GetStrokeStyle(idx, p); }

private:
    Ptr<ShapeDataFloat> pData;
};


}} // Scaleform::Render

#endif
