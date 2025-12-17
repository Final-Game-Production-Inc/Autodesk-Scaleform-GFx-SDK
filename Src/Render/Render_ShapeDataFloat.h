/**************************************************************************

PublicHeader:   Render
Filename    :   Render_ShapeDataFloat.h
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef SF_Render_ShapeDataFloat_H
#define SF_Render_ShapeDataFloat_H

#include "Kernel/SF_Array.h"
#include "Render_PathDataPacker.h"
#include "Render_ShapeDataDefs.h"

namespace Scaleform { namespace Render {


//------------------------------------------------------------------------
template<class ContainerType>
class ShapeDataFloatTempl : public ShapeDataInterface
{
    enum DataStatus
    {
        Status_Clean,
        Status_StartLayer,
        Status_StartPath,
        Status_MoveTo,
        Status_EdgeTo,
        Status_EndPath,
        Status_EndShape
    };

    enum DataTags
    {
        Tag_NewLayer,
        Tag_NewPath,
        Tag_MoveTo,
        Tag_LineTo,
        Tag_QuadTo,
        Tag_CubicTo,
        Tag_EndPath,
        Tag_EndShape
    };

public:
    ShapeDataFloatTempl(ContainerType* c, unsigned staringPos) :
        Status(Status_Clean),
        Fills(),
        Strokes(),
        Data(c),
        StartingPos(staringPos),
        StartX(0), StartY(0),
        LastX(0), LastY(0)
    {}

    void Clear()
    {
        Status = Status_Clean;
        Fills.Clear();
        Strokes.Clear();
        Data->Clear();
        StartX = 0; StartY = 0;
        LastX = 0; LastY = 0;
    }

    unsigned AddFillStyle(const FillStyleType& fill) 
    { 
        Fills.PushBack(fill); 
        return (unsigned)Fills.GetSize(); 
    }

    unsigned AddStrokeStyle(const StrokeStyleType& str) 
    { 
        Strokes.PushBack(str);
        return (unsigned)Strokes.GetSize(); 
    }

    unsigned AddStrokeStyle(float width, unsigned flags, float miter, UInt32 color)
    { 
        StrokeStyleType str = { width, 1.0f, flags, miter, color };
        Strokes.PushBack(str);
        return (unsigned)Strokes.GetSize(); 
    }

    void StartLayer()
    {
        SF_ASSERT(Status == Status_Clean || Status == Status_EndPath);
        PathDataEncoder<ContainerType> encoder(*Data);
        encoder.WriteChar(Tag_NewLayer);
        Status = Status_StartLayer;
    }

    void StartPath(unsigned leftStyle, unsigned rightStyle, unsigned strokeStyle)
    {
        SF_ASSERT(Status == Status_Clean || Status == Status_StartLayer || Status == Status_EndPath);
        PathDataEncoder<ContainerType> encoder(*Data);
        if (Status == Status_Clean)
            StartLayer();
        encoder.WriteChar(Tag_NewPath);
        encoder.WriteUInt30(leftStyle);
        encoder.WriteUInt30(rightStyle);
        encoder.WriteUInt30(strokeStyle);
        Status = Status_StartPath;
    }

    void MoveTo(float x, float y)
    {
        SF_ASSERT(Status == Status_StartPath);
        PathDataEncoder<ContainerType> encoder(*Data);
        encoder.WriteChar(Tag_MoveTo);
        encoder.WriteFloat(x);
        encoder.WriteFloat(y);
        Status = Status_MoveTo;
        StartX = x; StartY = y;
        LastX = x; LastY = y;
    }

    void LineTo(float x, float y)
    {
        SF_ASSERT(Status == Status_MoveTo || Status == Status_EdgeTo);
        PathDataEncoder<ContainerType> encoder(*Data);
        encoder.WriteChar(Tag_LineTo);
        encoder.WriteFloat(x);
        encoder.WriteFloat(y);
        Status = Status_EdgeTo;
        LastX = x; LastY = y;
    }

    void QuadTo(float cx, float cy, float ax, float ay)
    {
        SF_ASSERT(Status == Status_MoveTo || Status == Status_EdgeTo);
        PathDataEncoder<ContainerType> encoder(*Data);
        encoder.WriteChar(Tag_QuadTo);
        encoder.WriteFloat(cx);
        encoder.WriteFloat(cy);
        encoder.WriteFloat(ax);
        encoder.WriteFloat(ay);
        Status = Status_EdgeTo;
        LastX = ax; LastY = ay;
    }

    void CubicTo(float cx1, float cy1, float cx2, float cy2, float ax, float ay)
    {
        SF_ASSERT(Status == Status_MoveTo || Status == Status_EdgeTo);
        PathDataEncoder<ContainerType> encoder(*Data);
        encoder.WriteChar(Tag_CubicTo);
        encoder.WriteFloat(cx1);
        encoder.WriteFloat(cy1);
        encoder.WriteFloat(cx2);
        encoder.WriteFloat(cy2);
        encoder.WriteFloat(ax);
        encoder.WriteFloat(ay);
        Status = Status_EdgeTo;
        LastX = ax; LastY = ay;
    }

    void ClosePath()
    {
        if (LastX != StartX || LastY != StartY)
            LineTo(StartX, StartY);
    }

    void EndPath()
    {
        SF_ASSERT(Status == Status_MoveTo || Status == Status_EdgeTo);
        PathDataEncoder<ContainerType> encoder(*Data);
        encoder.WriteChar(Tag_EndPath);
        Status = Status_EndPath;
    }

    void EndShape()
    {
        if (Status != Status_EndShape)
        {
            if (Status == Status_Clean)
                return;

            SF_ASSERT(Status == Status_EdgeTo || Status == Status_EndPath);
            if (Status != Status_EndPath)
                EndPath();
            PathDataEncoder<ContainerType> encoder(*Data);
            encoder.WriteChar(Tag_EndShape);
            Status = Status_EndShape;
        }
    }

    virtual unsigned        GetFillStyleCount()   const { return (unsigned)Fills.GetSize(); }
    virtual unsigned        GetStrokeStyleCount() const { return (unsigned)Strokes.GetSize(); }
    virtual void            GetFillStyle(unsigned idx, FillStyleType* p) const { *p = Fills[idx-1]; }
    virtual void            GetStrokeStyle(unsigned idx, StrokeStyleType* p) const { *p = Strokes[idx-1]; }
    virtual unsigned        GetStartingPos() const { return StartingPos; }

    virtual ShapePathType   ReadPathInfo(ShapePosInfo* pos, float* coord, unsigned* styles) const
    {
        if (pos->Pos >= Data->GetSize())
            return Shape_EndShape;

        ShapePathType ret = Shape_NewPath;
        PathDataDecoder<ContainerType> decoder(*Data);
        char tag = decoder.ReadChar(pos->Pos); ++pos->Pos;

        if (tag == Tag_EndShape)
            return Shape_EndShape;

        if (tag == Tag_NewLayer)
        {
            ret = Shape_NewLayer;
            tag = decoder.ReadChar(pos->Pos); ++pos->Pos;
        }

        SF_ASSERT(tag == Tag_NewPath);
        pos->Pos += decoder.ReadUInt30(pos->Pos, &styles[0]);
        pos->Pos += decoder.ReadUInt30(pos->Pos, &styles[1]);
        pos->Pos += decoder.ReadUInt30(pos->Pos, &styles[2]);
        tag = decoder.ReadChar(pos->Pos); ++pos->Pos;
        SF_ASSERT(tag == Tag_MoveTo);
        coord[0] = decoder.ReadFloat(pos->Pos); pos->Pos += 4;
        coord[1] = decoder.ReadFloat(pos->Pos); pos->Pos += 4;
        return ret;
    }

    virtual PathEdgeType    ReadEdge(ShapePosInfo* pos, float* coord) const
    {
        PathEdgeType ret = Edge_EndPath;
        PathDataDecoder<ContainerType> decoder(*Data);
        char tag = decoder.ReadChar(pos->Pos); ++pos->Pos;
        if (tag != Tag_EndPath)
        {
            SF_ASSERT(tag == Tag_LineTo || tag == Tag_QuadTo || tag == Tag_CubicTo);
            ret = Edge_LineTo;
            coord[0] = decoder.ReadFloat(pos->Pos); pos->Pos += 4;
            coord[1] = decoder.ReadFloat(pos->Pos); pos->Pos += 4;
            if (tag == Tag_QuadTo || tag == Tag_CubicTo)
            {
                ret = Edge_QuadTo;
                coord[2] = decoder.ReadFloat(pos->Pos); pos->Pos += 4;
                coord[3] = decoder.ReadFloat(pos->Pos); pos->Pos += 4;
            }
            if (tag == Tag_CubicTo)
            {
                ret = Edge_CubicTo;
                coord[4] = decoder.ReadFloat(pos->Pos); pos->Pos += 4;
                coord[5] = decoder.ReadFloat(pos->Pos); pos->Pos += 4;
            }
        }
        return ret;
    }

    float GetLastX() const { return LastX; }
    float GetLastY() const { return LastY; }

private:
    DataStatus                  Status;
    ArrayLH<FillStyleType>      Fills;
    ArrayLH<StrokeStyleType>    Strokes;
    ContainerType*              Data;
    unsigned                    StartingPos;
    float                       StartX, StartY;
    float                       LastX, LastY;
};


//------------------------------------------------------------------------
class ShapeDataFloat : public ShapeDataFloatTempl<Array<UByte> >
{
    typedef ShapeDataFloatTempl<Array<UByte> > BaseType;
public:
    ShapeDataFloat() : BaseType(&Container, 0) {}

private:
    Array<UByte> Container;
};


}} // Scaleform::Render

#endif
