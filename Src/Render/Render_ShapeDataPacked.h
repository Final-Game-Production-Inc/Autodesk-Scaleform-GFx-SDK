/**************************************************************************

PublicHeader:   Render
Filename    :   Render_ShapeDataPacked.h
Content     :   
Created     :   2007
Authors     :   Maxim Shemanarev

Notes       :   Compact path data storage

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_ShapeDataPacked_H
#define INC_SF_Render_ShapeDataPacked_H

#include "Render_PathDataPacker.h"
#include "Render_ShapeDataDefs.h"

namespace Scaleform { namespace Render {


//------------------------------------------------------------------------
//    Stream data structure:
//
//    The stream consists of a number of paths. Each path begins with the header:
//    
//    Path header:
//    ---------------------
//    UInt15 PathFlag     : Shape_NewPath, Shape_NewLayer, or Shape_EndShape, see Render_ShapeDataDefs.h
//    UInt30 LeftStyle    : Left fill style, 0 means no fill
//    UInt30 RightStyle   : Right fill style, 0 means no fill
//    UInt30 StrokeStyle  : Stroke style, 0 means no stroke
//    SInt30 StartX       : Starting x coordinate
//    SInt30 StartY       : Starting y coordinate
//    ...Path data
//
//    Path data:
//    ---------------------
//    See Render_PathDataInt
//
//    Usage (example):
//
//    ShapeDataPackedDecoder<Array<UByte> > decoder(&dataArray, 20);
//    ShapePosInfo pos(startPos);
//    ShapePathType pathType;
//    float coords[Seg_MaxCoord];
//    unsigned styles[3];
//
//    while((pathType = decoder.ReadPathInfo(&pos, coords, styles)) != Shape_EndShape)
//    {
//        // coord contain MoveTo(x, y);
//        // styles contain LeftStyle, RightStyle, StrokeStyle
//        PathEdgeType edgeType;
//        while((segType = decoder.ReadSegment(&pos, coords)) != Edge_EndPath)
//        {
//            if(edgeType == Edge_LineTo)
//            {
//                // Line
//            }
//            else
//            if(edgeType == Edge_QuadTo)
//            {
//                // Quadratic Curve
//            }
//        }
//    }
//
//------------------------------------------------------------------------




//------------------------------------------------------------------------
template<class ContainerType> class ShapeDataPackedDecoder
{
public:
    ShapeDataPackedDecoder(const ContainerType* c, float multiplier) : 
        Decoder(*c), OneOverMultiplier(1.0f / multiplier) {}

    // coord must be float[Seg_MaxCoord], styles must be unsigned[3]
    //--------------------------------------------------------------------
    ShapePathType ReadPathInfo(ShapePosInfo* pos, float* coord, unsigned* styles) const
    {
        unsigned flag;
        pos->Pos += Decoder.ReadUInt15(pos->Pos, &flag);
        if (flag != Shape_EndShape)
        {
            pos->Pos += Decoder.ReadUInt30(pos->Pos, &styles[0]);
            pos->Pos += Decoder.ReadUInt30(pos->Pos, &styles[1]);
            pos->Pos += Decoder.ReadUInt30(pos->Pos, &styles[2]);
            pos->Pos += Decoder.ReadSInt30(pos->Pos, &pos->StartX);
            pos->Pos += Decoder.ReadSInt30(pos->Pos, &pos->StartY);
            pos->LastX = pos->StartX;
            pos->LastY = pos->StartY;
            coord[0] = float(pos->StartX) * OneOverMultiplier;
            coord[1] = float(pos->StartY) * OneOverMultiplier;
        }
        return ShapePathType(flag);
    }

    // coord must be float[Seg_MaxCoord]
    //--------------------------------------------------------------------
    PathEdgeType ReadEdge(ShapePosInfo* pos, float* coord) const
    {
        int tmp[5];
        pos->Pos += Decoder.ReadEdge(pos->Pos, tmp);
        switch(tmp[0])
        {
        case PathDataPackerTypes::Edge_HLine: 
            pos->LastX += tmp[1];
            coord[0] = float(pos->LastX) * OneOverMultiplier;
            coord[1] = float(pos->LastY) * OneOverMultiplier;
            return Edge_LineTo;

        case PathDataPackerTypes::Edge_VLine:
            pos->LastY += tmp[1];
            coord[0] = float(pos->LastX) * OneOverMultiplier;
            coord[1] = float(pos->LastY) * OneOverMultiplier;
            return Edge_LineTo;

        case PathDataPackerTypes::Edge_Line:
            pos->LastX += tmp[1];
            pos->LastY += tmp[2];
            coord[0] = float(pos->LastX) * OneOverMultiplier;
            coord[1] = float(pos->LastY) * OneOverMultiplier;
            return Edge_LineTo;

        case PathDataPackerTypes::Edge_Quad:
            tmp[1] += pos->LastX;
            tmp[2] += pos->LastY;
            tmp[3] += pos->LastX;
            tmp[4] += pos->LastY;
            coord[0] = float(tmp[1]) * OneOverMultiplier;
            coord[1] = float(tmp[2]) * OneOverMultiplier;
            coord[2] = float(tmp[3]) * OneOverMultiplier;
            coord[3] = float(tmp[4]) * OneOverMultiplier;
            pos->LastX = tmp[3];
            pos->LastY = tmp[4];
            return Edge_QuadTo;

        case PathDataPackerTypes::Edge_EndPath:
            break;
        }
        return Edge_EndPath;
    }

private:
    PathDataDecoder<ContainerType> Decoder; 
    float OneOverMultiplier;
};



//------------------------------------------------------------------------
//    Usage (example):
//
//    ShapeDataPackedEncoder<Array<UByte> > encoder(&dataArray, 20);
//    ShapePosInfo pos;
//
//    encoder.StartPath(&pos, Shape_NewLayer, leftStyle, rightStyle, strokeStyle, startX, startY);
//    while coord
//    {
//        encoder.LineTo(&pos, x, y);
//        . . .or:
//        encoder.QuadTo(&pos, cx, cy, ax, ay);
//    }
//    encoder.ClosePath(); // Optional
//    encoder.EndPath();
//
//    encoder.StartPath(&pos, Shape_NewPath, leftStyle, rightStyle, strokeStyle, startX, startY);
//    . . .etc
//    . . .
//    encoder.EndShape();
//------------------------------------------------------------------------


//------------------------------------------------------------------------
template<class ContainerType> class ShapeDataPackedEncoder
{
public:
    ShapeDataPackedEncoder(ContainerType* c, float multiplier) : 
        Encoder(*c), Multiplier(multiplier) {}

    // type: Shape_NewPath or Shape_NewLayer
    //--------------------------------------------------------------------
    void StartPath(ShapePosInfo* pos, ShapePathType type, 
                   unsigned leftStyle, unsigned rightStyle, unsigned strokeStyle,
                   float startX, float startY)
    {
        Encoder.WriteUInt15(type);
        Encoder.WriteUInt30(leftStyle);
        Encoder.WriteUInt30(rightStyle);
        Encoder.WriteUInt30(strokeStyle);
        Encoder.WriteSInt30(pos->StartX = pos->LastX = int(startX * Multiplier));
        Encoder.WriteSInt30(pos->StartY = pos->LastY = int(startY * Multiplier));
    }

    //--------------------------------------------------------------------
    void LineTo(ShapePosInfo* pos, float x, float y)
    {
        int dx = int(x * Multiplier) - pos->LastX;
        int dy = int(y * Multiplier) - pos->LastY;
             if(dy == 0) { Encoder.WriteHLine(dx); }
        else if(dx == 0) { Encoder.WriteVLine(dy); }
        else             { Encoder.WriteLine(dx, dy); }
        pos->LastX += dx;
        pos->LastY += dy;
    }

    //--------------------------------------------------------------------
    void QuadTo(ShapePosInfo* pos, float cx, float cy, float ax, float ay)
    {
        int dcx = int(cx * Multiplier) - pos->LastX;
        int dcy = int(cy * Multiplier) - pos->LastY;
        int dax = int(ax * Multiplier) - pos->LastX;
        int day = int(ay * Multiplier) - pos->LastY;
        Encoder.WriteQuad(dcx, dcy, dax, day);
        pos->LastX += dax;
        pos->LastY += day;
    }

    //------------------------------------------------------------------------
    void ClosePath(ShapePosInfo* pos)
    {
        if (pos->LastX != pos->StartX || pos->LastY != pos->StartY)
        {
            int dx = pos->StartX - pos->LastX;
            int dy = pos->StartY - pos->LastY;
                 if(dy == 0) { Encoder.WriteHLine(dx); }
            else if(dx == 0) { Encoder.WriteVLine(dy); }
            else             { Encoder.WriteLine(dx, dy); }
            pos->LastX += dx;
            pos->LastY += dy;
        }
    }

    //--------------------------------------------------------------------
    void EndPath()
    {
        Encoder.WriteEndPath();
    }

    //--------------------------------------------------------------------
    void EndShape()
    {
        Encoder.WriteUInt15(Shape_EndShape);
    }

private:
    PathDataEncoder<ContainerType> Encoder; 
    float Multiplier;
};




//------------------------------------------------------------------------
template<class ContainerType> class ShapeDataPackedNoStyles : public ShapeDataInterface
{
public:
    ShapeDataPackedNoStyles(ContainerType* c, unsigned startingPos, float multiplier) : 
        pContainer(c), Decoder(c, multiplier), StartingPos(startingPos), Multiplier(multiplier)
    {}

    // Encode shape data functions
    //---------------------------------------------------------------------
    void StartPath(ShapePosInfo* pos, ShapePathType type, 
                   unsigned leftStyle, unsigned rightStyle, unsigned strokeStyle,
                   float startX, float startY)
    {
        ShapeDataPackedEncoder<ContainerType> encoder(pContainer, Multiplier);
        encoder.StartPath(pos, type, leftStyle, rightStyle, strokeStyle, startX, startY);
    }

    void LineTo(ShapePosInfo* pos, float x, float y)
    {
        ShapeDataPackedEncoder<ContainerType> encoder(pContainer, Multiplier);
        encoder.LineTo(pos, x, y);
    }

    void QuadTo(ShapePosInfo* pos, float cx, float cy, float ax, float ay)
    {
        ShapeDataPackedEncoder<ContainerType> encoder(pContainer, Multiplier);
        encoder.QuadTo(pos, cx, cy, ax, ay);
    }

    void ClosePath(ShapePosInfo* pos)
    {
        ShapeDataPackedEncoder<ContainerType> encoder(pContainer, Multiplier);
        encoder.ClosePath(pos);
    }

    void EndPath()
    {
        ShapeDataPackedEncoder<ContainerType> encoder(pContainer, Multiplier);
        encoder.EndPath();
    }

    void EndShape()
    {
        ShapeDataPackedEncoder<ContainerType> encoder(pContainer, Multiplier);
        encoder.EndShape();
    }
    bool IsEmpty() const { return !pContainer || pContainer->GetSize() <= StartingPos; }


    // ShapeDataInterface overrides
    //---------------------------------------------------------------------
    virtual unsigned GetFillStyleCount() const { return 0; }
    virtual unsigned GetStrokeStyleCount() const { return 0; }
    virtual void GetFillStyle(unsigned, FillStyleType*) const {}
    virtual void GetStrokeStyle(unsigned, StrokeStyleType*) const {}

    // The implementation must provide some abstract index (position)
    // to quickly navigate to the beginning of the shape data. In simple case,
    // when the data is stored is an isolated array it can just return 0.
    virtual unsigned GetStartingPos() const
    {
        return StartingPos;
    }

    // Read path info. Returns Shape_NewPath, Shape_NewLayer, or Shape_EndShape and 
    // starting path info. Shape_NewLayer is the same as Shape_NewPath, but just
    // indicates that it's a new layer.
    // Coord must be at least float[2], styles must be unsigned[3].
    //--------------------------------------------------------------------
    virtual ShapePathType ReadPathInfo(ShapePosInfo* pos, float* coord, unsigned* styles) const
    {
        return Decoder.ReadPathInfo(pos, coord, styles);
    }

    // Read next edge. Returns Edge_LineTo, Edge_QuadTo, or Edge_EndPath. 
    // Coord must be float[Seg_MaxCoord].
    //--------------------------------------------------------------------
    virtual PathEdgeType ReadEdge(ShapePosInfo* pos, float* coord) const
    {
        return Decoder.ReadEdge(pos, coord);
    }

private:
    ContainerType*                          pContainer;
    ShapeDataPackedDecoder<ContainerType>   Decoder;
    unsigned                                StartingPos;
    float                                   Multiplier;
};






//------------------------------------------------------------------------
template<class ContainerType> class ShapeDataPacked : public ShapeDataInterface
{
public:
    ShapeDataPacked(ContainerType* c, unsigned startingPos, float multiplier) : 
        pContainer(c), Decoder(c, multiplier), StartingPos(startingPos), Multiplier(multiplier)
    {}

    // Encode shape data functions
    //---------------------------------------------------------------------
    unsigned AddFillStyle(const FillStyleType& fill) 
    { 
        FillStyles.PushBack(fill); return (unsigned)FillStyles.GetSize(); 
    }
    unsigned AddStrokeStyle(const StrokeStyleType& stroke) 
    { 
        StrokeStyles.PushBack(stroke); return (unsigned)StrokeStyles.GetSize(); 
    }
    void UpdateFillStyle(unsigned index, const FillStyleType& fill)
    {
        FillStyles[index-1] = fill;
    }
    void UpdateStrokeStyle(unsigned index, const StrokeStyleType& stroke)
    {
        StrokeStyles[index-1] = stroke;
    }

    void StartPath(ShapePosInfo* pos, ShapePathType type, 
                   unsigned leftStyle, unsigned rightStyle, unsigned strokeStyle,
                   float startX, float startY)
    {
        ShapeDataPackedEncoder<ContainerType> encoder(pContainer, Multiplier);
        encoder.StartPath(pos, type, leftStyle, rightStyle, strokeStyle, startX, startY);
    }

    void LineTo(ShapePosInfo* pos, float x, float y)
    {
        ShapeDataPackedEncoder<ContainerType> encoder(pContainer, Multiplier);
        encoder.LineTo(pos, x, y);
    }

    void QuadTo(ShapePosInfo* pos, float cx, float cy, float ax, float ay)
    {
        ShapeDataPackedEncoder<ContainerType> encoder(pContainer, Multiplier);
        encoder.QuadTo(pos, cx, cy, ax, ay);
    }

    void ClosePath(ShapePosInfo* pos)
    {
        ShapeDataPackedEncoder<ContainerType> encoder(pContainer, Multiplier);
        encoder.ClosePath(pos);
    }

    void EndPath()
    {
        ShapeDataPackedEncoder<ContainerType> encoder(pContainer, Multiplier);
        encoder.EndPath();
    }

    void EndShape()
    {
        ShapeDataPackedEncoder<ContainerType> encoder(pContainer, Multiplier);
        encoder.EndShape();
    }
    bool IsEmpty() const { return !pContainer || pContainer->GetSize() <= StartingPos; }


    // ShapeDataInterface overrides
    //---------------------------------------------------------------------
    virtual unsigned GetFillStyleCount() const
    {
        return (unsigned)FillStyles.GetSize();
    }

    virtual unsigned GetStrokeStyleCount() const
    {
        return (unsigned)StrokeStyles.GetSize();
    }

    // idx is in range [1...GetFillStyleCount()] (inclusive)
    virtual void GetFillStyle(unsigned idx, FillStyleType* p) const
    {
        if (idx > FillStyles.GetSize())
        {
            static FillStyleType defaultFillStyle = {0,0};
            *p = defaultFillStyle;
        }
        else
        {
            *p = FillStyles[idx-1];
        }
    }

    // idx is range [1...GetStrokeStyleCount()] (inclusive)
    virtual void GetStrokeStyle(unsigned idx, StrokeStyleType* p) const
    {
        if (idx > StrokeStyles.GetSize())
        {
            static StrokeStyleType defaultStrokeStyle = {0,0,0,0,0};
            *p = defaultStrokeStyle;
        }
        else
        {
            *p = StrokeStyles[idx-1];
        }
    }

    // The implementation must provide some abstract index (position)
    // to quickly navigate to the beginning of the shape data. In simple case,
    // when the data is stored is an isolated array it can just return 0.
    virtual unsigned GetStartingPos() const
    {
        return StartingPos;
    }

    // Read path info. Returns Shape_NewPath, Shape_NewLayer, or Shape_EndShape and 
    // starting path info. Shape_NewLayer is the same as Shape_NewPath, but just
    // indicates that it's a new layer.
    // Coord must be at least float[2], styles must be unsigned[3].
    //--------------------------------------------------------------------
    virtual ShapePathType ReadPathInfo(ShapePosInfo* pos, float* coord, unsigned* styles) const
    {
        return Decoder.ReadPathInfo(pos, coord, styles);
    }

    // Read next edge. Returns Edge_LineTo, Edge_QuadTo, or Edge_EndPath. 
    // Coord must be float[Seg_MaxCoord].
    //--------------------------------------------------------------------
    virtual PathEdgeType ReadEdge(ShapePosInfo* pos, float* coord) const
    {
        return Decoder.ReadEdge(pos, coord);
    }

private:
    ContainerType*                          pContainer;
    ShapeDataPackedDecoder<ContainerType>   Decoder;
    unsigned                                StartingPos;
    float                                   Multiplier;
    ArrayLH<FillStyleType>                  FillStyles;
    ArrayLH<StrokeStyleType>                StrokeStyles;
};




}} // Scaleform::Render

#endif
