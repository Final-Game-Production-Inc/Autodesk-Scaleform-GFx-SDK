/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Rasterizer.h
Content     :   
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Notes       :   Scanline rasterizer with anti-aliasing

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

----------------------------------------------------------------------
----The code of these classes was taken from the Anti-Grain Geometry
Project and modified for the use by Scaleform. 
Permission to use without restrictions is hereby granted to 
Scaleform Corporation by the author of Anti-Grain Geometry Project.
See http://antigtain.com for details.

The author of Anti-Grain Geometry gratefully acknowleges the support 
of David Turner, Robert Wilhelm, and Werner Lemberg - the authors of 
the FreeType libray - in producing this work. 
See http://www.freetype.org for details.
**************************************************************************/

#ifndef INC_SF_Render_Rasterizer_H
#define INC_SF_Render_Rasterizer_H

#include "Kernel/SF_Alg.h"
#include "Render_TessDefs.h"

#ifdef SF_MATH_H
#include SF_MATH_H
#else
#include <math.h>
#endif
#include <string.h>

namespace Scaleform { namespace Render {


class Rasterizer : public TessBase
{
public:
    enum
    {
        SubpixelShift = 8,
        SubpixelScale = 1 << SubpixelShift,
        SubpixelMask  = SubpixelScale - 1,

        AntiAliasShift  = 8,
        AntiAliasScale  = 1 << AntiAliasShift,
        AntiAliasMask   = AntiAliasScale - 1,
        AntiAliasScale2 = AntiAliasScale * 2,
        AntiAliasMask2  = AntiAliasScale2 - 1
    };

    enum FillRuleType
    {
        FillNonZero,
        FillEvenOdd
    };

    Rasterizer(MemoryHeap* heap);

    // Setup (optional)
    //-----------------------------------
    // FillRule = FillNonZero by default
    void            SetFillRule(FillRuleType f) { FillRule = f; }
    FillRuleType    GetFillRule() const         { return FillRule; }

    void            SetGamma1(float g);
    float           GetGamma1() const            { return Gamma1; }

    void            SetGamma2(float g);
    float           GetGamma2() const            { return Gamma2; }

    // TessDef interface
    //---------------------
    virtual void        Clear();
    void                MoveTo(float x, float y);
    void                LineTo(float x, float y);
    virtual void        AddVertex(CoordType x, CoordType y)  { LineTo(x, y); } // Alias
    virtual CoordType   GetLastX() const { return LastXf; }
    virtual CoordType   GetLastY() const { return LastYf; }
    virtual void        ClosePath();

    // TessDef compatibility
    //---------------------
    virtual void        FinalizePath(unsigned, unsigned, bool, bool) {}
    virtual void        Transform(const Matrix2F&) {}
    virtual Matrix2F    StretchTo(float, float, float, float) { return Matrix2F(); }
    virtual unsigned    GetVertexCount() const { return 0; }
    virtual unsigned    GetMeshCount() const { return 0; }
    virtual unsigned    GetMeshVertexCount(unsigned) const { return 0; }
    virtual unsigned    GetMeshTriangleCount(unsigned) const { return 0; }
    virtual void        GetMesh(unsigned, TessMesh*) const {}
    virtual unsigned    GetVertices(TessMesh*, TessVertex*, unsigned) const { return 0; }
    virtual void        GetTrianglesI16(unsigned, UInt16*, unsigned, unsigned) const {}

    // After AddShape() or MoveTo()/LineTo() the 
    // bounding box is valid, as well as the number of 
    // resulting scan lines.
    //-----------------------------------
    int         GetMinX() const { return MinX; }
    int         GetMinY() const { return MinY; }
    int         GetMaxX() const { return MaxX; }
    int         GetMaxY() const { return MaxY; }
    UPInt       GetNumScanlines() const { return SortedYs.GetSize(); }

    // Call SortCells() after the shape is added. It
    // returns true if there's something to sweep and 
    // false if the shape is empty.
    //-----------------------------------
    bool SortCells();

    // Sweep one scan line, that is, rasterize it. 
    // "pRaster" should point to the beginning of the row
    // of gray-scale pixels. The pRaster[0] corresponds to 
    // GetMinX() value. The typical loop is:
    //
    // if(rasterizer.SortCells())
    // {
    //     for(unsigned i = 0; i < rasterizer.NumScanlines(); i++)
    //     {
    //         rasterizer.SweepScanline(i, pRaster + i * rasterWidth);
    //     }
    // }
    //-----------------------------------
    void SweepScanline(unsigned scanline, unsigned char* raster,
                       unsigned numChannels = 1, int gammaIdx = 0) const;

    void SweepScanlineThreshold(unsigned scanline, unsigned char* raster,
                                unsigned numChannels = 1, 
                                unsigned threshold = AntiAliasMask/2) const;
private:
    struct Cell 
    { 
        int x, y, Cover, Area; 
    };

    struct SortedY 
    { 
        unsigned Start, Count; 
    };

    void setGamma(int idx, float g);
    void line(int x1, int y1, int x2, int y2);
    void horLine(int ey, int x1, int y1, int x2, int y2);

    SF_INLINE void setCurrCell(int x, int y)
    {
        if((CurrCell.x - x) | (CurrCell.y - y))
        {
            if(CurrCell.Area | CurrCell.Cover)
            {
                Cells.PushBack(CurrCell);
            }
            CurrCell.x     = x;
            CurrCell.y     = y;
            CurrCell.Cover = 0;
            CurrCell.Area  = 0;
        }
    }

    SF_INLINE static bool cellXLess(const Cell* a, const Cell* b)
    {
        return a->x < b->x;
    }

    SF_INLINE unsigned calcAlpha(int area, int gammaIdx) const
    {
        int alpha = area >> (SubpixelShift * 2 + 1 - AntiAliasShift);

        if (alpha < 0) 
            alpha = -alpha;
        if (FillRule == FillEvenOdd)
        {
            alpha &= AntiAliasMask2;
            if (alpha > AntiAliasScale)
                alpha = AntiAliasScale2 - alpha;
        }
        if (alpha > AntiAliasMask) 
            alpha = AntiAliasMask;
        return GammaLut[gammaIdx][alpha];
    }
    
    SF_INLINE unsigned calcAlpha(int area, unsigned threshold) const
    {
        int alpha = area >> (SubpixelShift * 2 + 1 - AntiAliasShift);

        if (alpha < 0) 
            alpha = -alpha;
        if (FillRule == FillEvenOdd)
        {
            alpha &= AntiAliasMask2;
            if (alpha > AntiAliasScale)
                alpha = AntiAliasScale2 - alpha;
        }
        return (alpha <= int(threshold)) ? 0 : AntiAliasMask;
    }

    LinearHeap                  LHeap;
    FillRuleType                FillRule;
    float                       Gamma1;
    float                       Gamma2;
    UByte                       GammaLut[2][256];
    ArrayPaged<Cell, 4, 16>     Cells;
    ArrayUnsafe<Cell*>          SortedCells;
    ArrayUnsafe<SortedY>        SortedYs;
    Cell                        CurrCell;
    int                         MinX;
    int                         MinY;
    int                         MaxX;
    int                         MaxY;
    int                         StartX;
    int                         StartY;
    int                         LastX;
    int                         LastY;
    float                       LastXf;
    float                       LastYf;
};


}} // Scaleform::Render


#endif
