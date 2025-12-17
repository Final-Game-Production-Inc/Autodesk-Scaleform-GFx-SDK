/**************************************************************************

Filename    :   Render_Rasterizer.cpp
Content     :   
Created     :   2005
Authors     :   Maxim Shemanarev

Notes       :   Scanline rasterizer with anti-aliasing

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

----------------------------------------------------------------------
The code of these classes was taken from the Anti-Grain Geometry
Project and modified for the use by Scaleform/Autodesk. 
Permission to use without restrictions is hereby granted to 
Scaleform/Autodesk by the author of Anti-Grain Geometry Project.
See http://antigrain.com for details.
**************************************************************************/

#include "Render_Rasterizer.h"

namespace Scaleform { namespace Render {

//------------------------------------------------------------------------
Rasterizer::Rasterizer(MemoryHeap* heap) :
    LHeap(heap),
    FillRule(FillNonZero),
    Gamma1(1.0f),
    Gamma2(1.0f),
    Cells(&LHeap),
    SortedCells(&LHeap),
    SortedYs(&LHeap),
    CurrCell(),
    MinX(0x7FFFFFFF),
    MinY(0x7FFFFFFF),
    MaxX(-0x7FFFFFFF),
    MaxY(-0x7FFFFFFF),
    StartX(0),
    StartY(0),
    LastX(0),
    LastY(0),
    LastXf(0),
    LastYf(0)
{
    CurrCell.x = 0x7FFFFFFF;
    CurrCell.y = 0x7FFFFFFF;
    CurrCell.Cover = 0;
    CurrCell.Area  = 0;
    SetGamma1(1.0f);
    SetGamma2(1.0f);
}

//------------------------------------------------------------------------
void Rasterizer::Clear()
{
    Cells.ClearAndRelease();
    SortedCells.ClearAndRelease();
    SortedYs.ClearAndRelease();
    LHeap.ClearAndRelease();
    CurrCell.x = 0x7FFFFFFF;
    CurrCell.y = 0x7FFFFFFF;
    CurrCell.Cover = 0;
    CurrCell.Area  = 0;
    MinX = 0x7FFFFFFF;
    MinY = 0x7FFFFFFF;
    MaxX = -0x7FFFFFFF;
    MaxY = -0x7FFFFFFF;
    StartX = 0;
    StartY = 0;
    LastX  = 0;
    LastY  = 0;
    LastXf = 0;
    LastYf = 0;
}

//------------------------------------------------------------------------
void Rasterizer::setGamma(int idx, float g)
{
    for(int i = 0; i < AntiAliasScale; i++)
    {
        GammaLut[idx][i] = UByte(pow(float(i) / AntiAliasMask, g) * AntiAliasMask + float(0.5));
    }
}


//------------------------------------------------------------------------
void Rasterizer::SetGamma1(float g)
{
    setGamma(0, g);
    Gamma1 = g;
}

//------------------------------------------------------------------------
void Rasterizer::SetGamma2(float g)
{
    setGamma(1, g);
    Gamma2 = g;
}

//------------------------------------------------------------------------
void Rasterizer::MoveTo(float x, float y)
{
    LastXf = x;
    LastYf = y;
    StartX = LastX = int(x * SubpixelScale);
    StartY = LastY = int(y * SubpixelScale);
}

//------------------------------------------------------------------------
void Rasterizer::LineTo(float x, float y)
{
    LastXf = x;
    LastYf = y;
    int xi = int(x * SubpixelScale);
    int yi = int(y * SubpixelScale);
    line(LastX, LastY, xi, yi);
    LastX = xi;
    LastY = yi;
}

//------------------------------------------------------------------------
void Rasterizer::ClosePath()
{
    if (LastX != StartX || LastY != StartY)
    {
        line(LastX, LastY, StartX, StartY);
        LastX = StartX;
        LastY = StartY;
    }
}

//------------------------------------------------------------------------
void Rasterizer::horLine(int ey, int x1, int y1, int x2, int y2)
{
    int ex1 = x1 >> SubpixelShift;
    int ex2 = x2 >> SubpixelShift;
    int fx1 = x1 &  SubpixelMask;
    int fx2 = x2 &  SubpixelMask;

    int delta, p, first, dx;
    int incr, lift, mod, rem;

    // Trivial case. Happens often
    if(y1 == y2) 
    {
        setCurrCell(ex2, ey);
        return;
    }

    // Everything is located in a single cell. That is easy!
    if(ex1 == ex2)
    {
        delta = y2 - y1;
        CurrCell.Cover += delta;
        CurrCell.Area  += (fx1 + fx2) * delta;
        return;
    }

    // OK, we'll have to render a run of adjacent cells 
    // on the same HorLine...
    p     = (SubpixelScale - fx1) * (y2 - y1);
    first = SubpixelScale;
    incr  = 1;

    dx = x2 - x1;

    if(dx < 0)
    {
        p     = fx1 * (y2 - y1);
        first = 0;
        incr  = -1;
        dx    = -dx;
    }

    delta = p / dx;
    mod   = p % dx;

    if(mod < 0)
    {
        delta--;
        mod += dx;
    }

    CurrCell.Cover += delta;
    CurrCell.Area  += (fx1 + first) * delta;

    ex1 += incr;
    setCurrCell(ex1, ey);
    y1  += delta;

    if(ex1 != ex2)
    {
        p     = SubpixelScale * (y2 - y1 + delta);
        lift  = p / dx;
        rem   = p % dx;

        if (rem < 0)
        {
            lift--;
            rem += dx;
        }

        mod -= dx;

        while(ex1 != ex2)
        {
            delta = lift;
            mod  += rem;
            if(mod >= 0)
            {
                mod -= dx;
                delta++;
            }

            CurrCell.Cover += delta;
            CurrCell.Area  += SubpixelScale * delta;

            y1  += delta;
            ex1 += incr;
            setCurrCell(ex1, ey);
        }
    }
    delta = y2 - y1;
    CurrCell.Cover += delta;
    CurrCell.Area  += (fx2 + SubpixelScale - first) * delta;
}

//------------------------------------------------------------------------
void Rasterizer::line(int x1, int y1, int x2, int y2)
{
    int dx  = x2 - x1;
    int dy  = y2 - y1;
    int ex1 = x1 >> SubpixelShift;
    int ey1 = y1 >> SubpixelShift;
    int ex2 = x2 >> SubpixelShift;
    int ey2 = y2 >> SubpixelShift;
    int fy1 = y1 &  SubpixelMask;
    int fy2 = y2 &  SubpixelMask;

    int xFrom, xTo;
    int p, rem, mod, lift, delta, first, incr;

    if(ex1 < MinX) MinX = ex1;
    if(ex1 > MaxX) MaxX = ex1;
    if(ey1 < MinY) MinY = ey1;
    if(ey1 > MaxY) MaxY = ey1;
    if(ex2 < MinX) MinX = ex2;
    if(ex2 > MaxX) MaxX = ex2;
    if(ey2 < MinY) MinY = ey2;
    if(ey2 > MaxY) MaxY = ey2;

    setCurrCell(ex1, ey1);
    
    // Everything is on a single HorLine
    if(ey1 == ey2)
    {
        horLine(ey1, x1, fy1, x2, fy2);
        return;
    }

    // Vertical line - we have to calculate start and end cells,
    // and then - the common values of the area and coverage for
    // all cells of the line. We know exactly there's only one 
    // cell, so, we don't have to call render_hline().
    incr  = 1;
    if(dx == 0)
    {
        int ex = x1 >> SubpixelShift;
        int twoFx = (x1 - (ex << SubpixelShift)) << 1;
        int area;

        first = SubpixelScale;
        if(dy < 0)
        {
            first = 0;
            incr  = -1;
        }

        delta = first - fy1;
        CurrCell.Cover += delta;
        CurrCell.Area  += twoFx * delta;

        ey1 += incr;
        setCurrCell(ex, ey1);
   
        delta = first + first - SubpixelScale;
        area  = twoFx * delta;
        while(ey1 != ey2)
        {
            CurrCell.Cover = delta;
            CurrCell.Area  = area;
            ey1 += incr;
            setCurrCell(ex, ey1);
        }

        delta = fy2 - SubpixelScale + first;
        CurrCell.Cover += delta;
        CurrCell.Area  += twoFx * delta;
        return;
    }

    // OK, we have to render several HorLines
    p     = (SubpixelScale - fy1) * dx;
    first = SubpixelScale;

    if(dy < 0)
    {
        p     = fy1 * dx;
        first = 0;
        incr  = -1;
        dy    = -dy;
    }

    delta = p / dy;
    mod   = p % dy;

    if(mod < 0)
    {
        delta--;
        mod += dy;
    }

    xFrom = x1 + delta;
    horLine(ey1, x1, fy1, xFrom, first);

    ey1 += incr;
    setCurrCell(xFrom >> SubpixelShift, ey1);

    if(ey1 != ey2)
    {
        p     = SubpixelScale * dx;
        lift  = p / dy;
        rem   = p % dy;

        if(rem < 0)
        {
            lift--;
            rem += dy;
        }
        mod -= dy;

        while(ey1 != ey2)
        {
            delta = lift;
            mod  += rem;
            if (mod >= 0)
            {
                mod -= dy;
                delta++;
            }

            xTo = xFrom + delta;
            horLine(ey1, xFrom, SubpixelScale - first, xTo, first);
            xFrom = xTo;

            ey1 += incr;
            setCurrCell(xFrom >> SubpixelShift, ey1);
        }
    }
    horLine(ey1, xFrom, SubpixelScale - first, x2, fy2);
}

//------------------------------------------------------------------------
bool Rasterizer::SortCells()
{
    if(CurrCell.Area | CurrCell.Cover)
    {
        Cells.PushBack(CurrCell);
    }
    CurrCell.x     = 0x7FFFFFFF;
    CurrCell.y     = 0x7FFFFFFF;
    CurrCell.Cover = 0;
    CurrCell.Area  = 0;

    if(Cells.GetSize() == 0) 
        return false; // Empty 
    if(SortedYs.GetSize()) 
        return true;  // Already sorted.

    // *** Perform sort.

    // Allocate the array of cell pointers
    SortedCells.Resize(Cells.GetSize());

    // Allocate and zero the Y array
    SortedYs.Resize(MaxY - MinY + 1);
    SortedYs.Zero();

    unsigned i;

    // Create the Y-histogram (count the numbers of cells for each Y)
    for(i = 0; i < Cells.GetSize(); i++)
    {
        SortedYs[Cells[i].y - MinY].Start++;
    }

    // Convert the Y-histogram into the array of starting indexes
    unsigned start = 0;
    for(i = 0; i < SortedYs.GetSize(); i++)
    {
        unsigned v = SortedYs[i].Start;
        SortedYs[i].Start = start;
        start += v;
    }

    // Fill the cell pointer array sorted by Y
    for(i = 0; i < Cells.GetSize(); i++)
    {
        Cell*       pcell = &Cells[i];
        SortedY&    currY = SortedYs[pcell->y - MinY];
        SortedCells[currY.Start + currY.Count] = pcell;
        ++currY.Count;
    }

    // Finally arrange the X-arrays
    for(i = 0; i < SortedYs.GetSize(); i++)
    {
        const SortedY& currY = SortedYs[i];
        if(currY.Count)
        {
            Alg::ArrayAdaptor<Cell*> sortedCells(&SortedCells[currY.Start], currY.Count);
            Alg::QuickSort(sortedCells, cellXLess);
        }
    }
    return true;
}

//------------------------------------------------------------------------
void Rasterizer::SweepScanline(unsigned scanline, unsigned char* pRaster,
                               unsigned numChannels, int gammaIdx) const
{
    if(scanline >= SortedYs.GetSize()) return;

    unsigned numCells = SortedYs[scanline].Count;
    const Cell* const* cells = &SortedCells[SortedYs[scanline].Start];
    int cover = 0;

    while(numCells)
    {
        const Cell* currCell = *cells;
        int x    = currCell->x;
        int area = currCell->Area;
        unsigned alpha;

        cover += currCell->Cover;

        //accumulate all cells with the same X
        while(--numCells)
        {
            currCell = *++cells;
            if(currCell->x != x) 
                break;
            area  += currCell->Area;
            cover += currCell->Cover;
        }

        if(area)
        {
            alpha = calcAlpha((cover << (SubpixelShift + 1)) - area, gammaIdx);
            UByte* p = pRaster + (x - MinX) * numChannels;
            for (unsigned j = 0; j < numChannels; j++) 
                *p++ = UByte(alpha);
            x++;
        }

        if(numCells && currCell->x > x)
        {
            alpha = calcAlpha(cover << (SubpixelShift + 1), gammaIdx);
            if (alpha)
            {
                memset(pRaster + (x - MinX) * numChannels, 
                       alpha, 
                       (currCell->x - x) * numChannels);
            }
        }
    }
}


//------------------------------------------------------------------------
void Rasterizer::SweepScanlineThreshold(unsigned scanline, unsigned char* pRaster,
                                        unsigned numChannels, unsigned threshold) const
{
    if(scanline >= SortedYs.GetSize()) return;

    unsigned numCells = SortedYs[scanline].Count;
    const Cell* const* cells = &SortedCells[SortedYs[scanline].Start];
    int cover = 0;

    while(numCells)
    {
        const Cell* currCell = *cells;
        int x    = currCell->x;
        int area = currCell->Area;
        unsigned alpha;

        cover += currCell->Cover;

        //accumulate all cells with the same X
        while(--numCells)
        {
            currCell = *++cells;
            if(currCell->x != x) 
                break;
            area  += currCell->Area;
            cover += currCell->Cover;
        }

        if(area)
        {
            alpha = calcAlpha((cover << (SubpixelShift + 1)) - area, threshold);
            if(alpha)
            {
                UByte* p = pRaster + (x - MinX) * numChannels;
                for (unsigned j = 0; j < numChannels; j++) 
                    *p++ = UByte(alpha);
            }
            x++;
        }

        if(numCells && currCell->x > x)
        {
            alpha = calcAlpha(cover << (SubpixelShift + 1), threshold);
            if (alpha)
            {
                memset(pRaster + (x - MinX) * numChannels, 
                       alpha, 
                       (currCell->x - x) * numChannels);
            }
        }
    }
}

}} //namespace Scaleform { namespace Render {
