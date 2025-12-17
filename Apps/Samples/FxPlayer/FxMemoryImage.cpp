/**************************************************************************

Filename    :   FxMemoryImage.cpp
Content     :   Memory image to be displayed as a texture
                
Created     :   2009
Authors     :   Maxim Shemenarev, Boris Rayskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxMemoryImage.h"
#include "GHeapTypes.h"
#include "FxFontProviderHUD.h"
#include "Kernel/SF_MsgFormat.h"


const unsigned FxMemoryImage::WidthRamp[]  = { 16, 16, 8, 8, 4, 4, 2, 2, 1 };
const unsigned FxMemoryImage::HeightRamp[] = { 16, 8,  8, 4, 4, 2, 2, 1, 1 };


//------------------------------------------------------------------------
FxMemoryImage::FxMemoryImage() :
    Background(0),
    Segments(),
    ImageBuffer(0),
    Image(),
    SizeRampIdx(0),
    PrevRampIdx(0),
    NumChartBars(0),
    BarHeight(0),
    SizeShift(0),
    PrevShift(0),
    MemHistory(0),
    LastStart(0),
    HistoryIdx(0),
    NumCharts(0),
    SteadyCount(0),
    UpdateDutyFactor(20)
{

}


void FxMemoryImage::SetHeapColors(const UInt32* heapsColors,    UInt32 numHeaps, 
                                  const UInt32* sysHeapsColors, UInt32 numSysHeaps)
{
    FxImportantHeapsColors = heapsColors;
    NumHeaps = numHeaps;

    FxSysHeapColors = sysHeapsColors;
    NumSysHeaps = numSysHeaps;
}


//------------------------------------------------------------------------
void FxMemoryImage::Create(UInt32 width, UInt32 height, UInt32 background)
{
    Background      = background;
    ImageBuffer     = 0;
    Image           = R30::ImageBase(R30::Image::Image_ARGB_8888, width, height, width * 4);
    Image.pData     = 0;
    Image.DataSize  = (unsigned)(width * height * 4);
}

//------------------------------------------------------------------------
FxMemoryImage::~FxMemoryImage()
{
    if (MemHistory)
        MemoryHeap::FreeSysDirect(MemHistory, Image.Width * MaxCharts * sizeof(FxMemHistory));

    if (ImageBuffer) 
        MemoryHeap::FreeSysDirect(ImageBuffer, Image.DataSize);
}


//------------------------------------------------------------------------
void FxMemoryImage::ReleaseMemory()
{
    Segments.ClearAndRelease();
    Heaps.ClearAndRelease();

    if (ImageBuffer) 
    {
        MemoryHeap::FreeSysDirect(ImageBuffer, Image.DataSize);
    }
    ImageBuffer = Image.pData = 0;
}


//------------------------------------------------------------------------
void FxMemoryImage::ClearImage()
{
    if (ImageBuffer == 0)
    {
        ImageBuffer = (UByte *)MemoryHeap::AllocSysDirect(Image.DataSize);
        Image.pData = ImageBuffer;
    }
    for(UInt32 y = 0; y < Image.Height; ++y)
    {
        UInt32* pix = (UInt32 *)Image.GetScanline(y);
        for(UInt32 x = 0; x < Image.Width; ++x)
        {
            pix[x] = Background;
        }
    }
}


//------------------------------------------------------------------------
void FxMemoryImage::drawChar(unsigned x, unsigned y, UInt32 c, const UByte* bits)
{
    unsigned w = GFxExternalFontHUD::GetMonoFontWidth();
    unsigned h = GFxExternalFontHUD::GetMonoFontHeight();
    unsigned b = GFxExternalFontHUD::GetMonoFontBaseLine();
    for(unsigned i = 0; i < h; ++i)
    {
        unsigned yl = y - b + i;
        if (yl < Image.Height)
        {
            UByte mask = bits[i];
            UInt32* sl = (UInt32*)Image.GetScanline(yl);
            for (unsigned j = 0; j < w; ++j)
            {
                unsigned xp = x + j;
                if (xp < Image.Width && (mask & 0x80))
                {
                    sl[xp] = c;
                }
                mask <<= 1;
            }
        }
    }
}

//------------------------------------------------------------------------
void FxMemoryImage::drawChar(unsigned x, unsigned y, UInt32 c1, UInt32 c2, unsigned code)
{
    const UByte* bits = GFxExternalFontHUD::GetGlyphBits(UInt16(code));
    if (c1)
    {
        drawChar(x-1, y,   c1, bits);
        drawChar(x+1, y,   c1, bits);
        drawChar(x,   y-1, c1, bits);
        drawChar(x,   y+1, c1, bits);
        //drawChar(x-1, y-1, c1, bits);
        //drawChar(x+1, y+1, c1, bits);
        //drawChar(x+1, y-1, c1, bits);
        //drawChar(x-1, y+1, c1, bits);
    }
    drawChar(x, y, c2, bits);
}


//------------------------------------------------------------------------
void FxMemoryImage::drawText(unsigned x, unsigned y, UInt32 c1, UInt32 c2, const char* text)
{

#if defined(SF_OS_PS3) || defined(SF_OS_WII)

    c2 = (c2 << 8) | 0xFF;

#endif

    const char* t = text;
    unsigned ch;
    while((ch = UTF8Util::DecodeNextChar(&t)) != 0)
    {
        drawChar(x, y, c1, c2, ch);
        x += GFxExternalFontHUD::GetMonoFontWidth();
    }
}


//------------------------------------------------------------------------
void FxMemoryImage::addSegment(unsigned type, const MemoryHeap* heap, 
                               UPInt addr, UPInt size)
{
    FxMemSegment seg;
    seg.Addr = addr;
    seg.Size = size;
    seg.Heap = heap;
    seg.Type = UByte(type);
    seg.NextSqueezed = false;
    Segments.PushBack(seg);
}


//------------------------------------------------------------------------
UPInt FxMemoryImage::calcSqueezeSize()
{
    UPInt squeeze   = 0;
    UPInt rowSpace  = GetRowAddrSpace();
    UPInt base      = Segments[0].Addr;
    for (UPInt i = 1; i < Segments.GetSize(); ++i)
    {
        UPInt start = Segments[i - 1].Addr + Segments[i - 1].Size - base;
        UPInt end   = Segments[i].Addr - base;
        start = (start + rowSpace - 1) / rowSpace * rowSpace;
        end   =  end / rowSpace * rowSpace;
        if (end > start + rowSpace)
        {
            squeeze += end - start - rowSpace;
        }
    }
    return squeeze;
}


//------------------------------------------------------------------------
bool FxMemoryImage::squeezeBiggestGap()
{
    UPInt rowSpace  = GetRowAddrSpace();
    UPInt base      = Segments[0].Addr;
    UPInt startIdx  = ~UPInt(0);
    UPInt squeeze   = 0;
    UPInt i;
    for (i = 1; i < Segments.GetSize(); ++i)
    {
        if (Segments[i - 1].NextSqueezed)
        {
            continue;
        }
        UPInt start = Segments[i - 1].Addr + Segments[i - 1].Size - base;
        UPInt end   = Segments[i].Addr - base;
        start = (start + rowSpace - 1) / rowSpace * rowSpace;
        end   =  end / rowSpace * rowSpace;
        if (end > start + rowSpace)
        {
            if (squeeze < end - start - rowSpace)
            {
                squeeze  = end - start - rowSpace;
                startIdx = i - 1;
            }
        }
    }
    if (squeeze == 0)
    {
        return false;
    }
    Segments[startIdx].NextSqueezed = true;
    for (i = startIdx + 1; i < Segments.GetSize(); ++i)
    {
        Segments[i].Addr -= squeeze;
    }
    return true;
}


//------------------------------------------------------------------------
UInt32 FxMemoryImage::getColor(UPInt type, const MemoryHeap* heap) const
{
    if (heap)
    {
        if(heap->GetFlags() & MemoryHeap::Heap_UserDebug)
        {
            return FxImportantHeapsColors[HeapId_HUDHeaps * 2 - 1];
        }

        return FxImportantHeapsColors[heap->GetId() * 2 - 1];
    }
    else
    {
        switch(type & ~GHeapSegVisitor::Seg_UnusedMask)
        {
        case GHeapSegVisitor::Seg_SysMem:      return FxSysHeapColors[0];
        //case GHeapSegVisitor::Seg_PageMap:     return FxSysHeapColors[1];
        case GHeapSegVisitor::Seg_Bookkeeper:  return FxSysHeapColors[2];
        case GHeapSegVisitor::Seg_DebugInfo:   return FxSysHeapColors[1];
        }
    }
    return FxImportantHeapsColors[HeapId_OtherHeaps * 2 - 1];
}


//------------------------------------------------------------------------
void FxMemoryImage::drawSegment(UPInt start, UPInt end, UInt32 color, bool unused)
{
    UPInt cellW = WidthRamp[SizeRampIdx];
    UPInt cellH = HeightRamp[SizeRampIdx];

    bool   drawVer = cellW >= 4 && !unused;
    UInt32 gradient = 64;
    UInt32 dgX = gradient / (UInt32)cellW;
    UInt32 dgY = gradient / (UInt32)cellH;
    UInt32 du  = gradient * (unused && cellW < 4);
    for(; start < end; ++start)
    {
        UPInt startX = (start % (Image.Width / cellW)) * cellW;
        UPInt startY = (start / (Image.Width / cellW)) * cellH;

        if (startY >= Image.Height)
        {
            break;
        }

        UInt32 x, y;
        for (y = 0; y < cellH; ++y)
        {
            UInt32* sl = (UInt32 *)(Image.GetScanline((unsigned)(startY + y)) + startX * 4);
            UInt32 r = du + y * dgY + ((color >> 16) & 0xFF); if (r > 0xFF) r = 0xFF;
            UInt32 g = du + y * dgY + ((color >> 8 ) & 0xFF); if (g > 0xFF) g = 0xFF;
            UInt32 b = du + y * dgY + ((color      ) & 0xFF); if (b > 0xFF) b = 0xFF;

#if defined(SF_OS_LINUX)

            {
                UInt32 tmp = r;
                r = b;
                b = tmp;
            }

            UInt32 c = 0xFF000000 | (r << 16) | (g << 8) | b;

#elif defined(SF_OS_PS3)

            UInt32 c = (r << 24) | (g << 16) | (b << 8) | 0xFF;
       
#elif defined(SF_OS_WII)

			UInt32 c = (r << 24) | (g << 16) | (b << 8) | 0xFF;

#else
            UInt32 c = 0xFF000000 | (r << 16) | (g << 8) | b;
            
#endif

            if (unused)
            {
                UPInt pixW = (cellW >= 4 && start+1 == end) ? cellW-1 : cellW;
                for(x = 0; x < pixW; ++x)
                {
                    r = x * dgX + ((c >> 16) & 0xFF); if (r > 0xFF) r = 0xFF;
                    g = x * dgX + ((c >> 8 ) & 0xFF); if (g > 0xFF) g = 0xFF;
                    b = x * dgX + ((c      ) & 0xFF); if (b > 0xFF) b = 0xFF;
                    sl[x] = 0xFF000000 | (r << 16) | (g << 8) | b;
                }
            }
            else
            {
                for(x = 0; x < cellW; ++x)
                {
                    sl[x] = c;
                }
            }
            if (drawVer && start + 1 == end)
            {
                sl[cellW - 1] = ((c >> 1) & 0xFF7F7F7F) | 0xFF000000;
            }
        }
    }
}


//------------------------------------------------------------------------
void FxMemoryImage::drawSqueezedRow(UPInt start)
{
    UPInt cellW = WidthRamp[SizeRampIdx];
    UPInt cellH = HeightRamp[SizeRampIdx];

    UPInt startY = (start / (Image.Width / cellW)) * cellH;
    if (startY >= Image.Height)
    {
        return;
    }

    UInt32 colors[2] = { 0xFF000000, 0xFFFFFFFF };
    UPInt x, y;
    for (y = 0; y < cellH; ++y)
    {
        UInt32* sl = (UInt32 *)Image.GetScanline((unsigned)(startY + y));
        for(x = 0; x < Image.Width; ++x)
        {
            UInt32 c = colors[(x & 1) ^ (y & 1)];
            sl[x] = c;
        }
    }
}


//------------------------------------------------------------------------
void FxMemoryImage::CreateMemoFrag(SqueezeOption sqOpt, int fixedSize)
{
    ClearImage();

    Segments.ClearAndRelease();
    FxMemSegVisitor visitor(this);
    MemoryHeap::VisitRootSegments(&visitor);
    
    visitSegments(Memory::pGlobalHeap, &visitor);
    Alg::QuickSort(Segments, cmpSegAddr);

    enum { RampSize = sizeof(WidthRamp) / sizeof(WidthRamp[0]) };

    if (fixedSize >= 0)
    {
        SizeRampIdx = (fixedSize >= RampSize) ? RampSize - 1 : fixedSize;
    }
    else
    {
        for(SizeRampIdx = 0; SizeRampIdx < RampSize - 1; ++SizeRampIdx)
        {
            UPInt squeeze = (sqOpt == DontSqueeze) ? 0 : calcSqueezeSize();
            UPInt range = Segments[Segments.GetSize() - 1].Addr + 
                          Segments[Segments.GetSize() - 1].Size - 
                          Segments[0].Addr;
            UPInt totalSpace = GetTotalAddrSpace();

            // The following logic provides a hysteresis; that is, the function
            // switches back to a bigger size only if there's a 25% difference.
            UPInt extra = (SizeRampIdx < PrevRampIdx) ? totalSpace / 4 : 0;
            if (totalSpace >= range - squeeze + extra)
            {
                break;
            }
        }
    }

    if (sqOpt != DontSqueeze)
    {
        for (;;)
        {
            UPInt range = Segments[Segments.GetSize() - 1].Addr + 
                          Segments[Segments.GetSize() - 1].Size - 
                          Segments[0].Addr;

            if (sqOpt != AlwaysSqueeze && GetTotalAddrSpace() >= range)
            {
                break;
            }
            if (!squeezeBiggestGap())
            {
                break;
            }
        }
    }

    //UPInt memSize = 0; // DBG
    UPInt base = Segments[0].Addr;
    for (UPInt i = 0; i < Segments.GetSize(); ++i)
    {
        UPInt  start    = (Segments[i].Addr - base) / GetPageSize();
        UPInt  size     =  Segments[i].Size / GetPageSize();
        UInt32 color    = getColor(Segments[i].Type, Segments[i].Heap);
        bool   unused   = (Segments[i].Type & GHeapSegVisitor::Seg_UnusedMask) != 0;
        drawSegment(start, start + size, color, unused);
        if (Segments[i].NextSqueezed)
        {
            UPInt rowSpace  = GetRowAddrSpace();
            start           = Segments[i].Addr + Segments[i].Size - base;
            start           = ((start + rowSpace - 1) / rowSpace * rowSpace) / GetPageSize();
            drawSqueezedRow(start);
        }
        //memSize += Segments[i].Size; // DBG
    }
    PrevRampIdx = SizeRampIdx;

    SF::StringBuffer textStr1;
    Format(
        textStr1,
        "Row address space: {0:sep:,}K; " 
        "Total address space: {1:sep:,}K; " 
        "Page size: {2:sep:,}K",
        (GetRowAddrSpace()      + 1023) / 1024,
        (GetTotalAddrSpace()    + 1023) / 1024,
        (GetPageSize()          + 1023) / 1024
        );

    drawText((unsigned)(5), (unsigned)(Image.Height - 10), 0xFFFFFFFF, 0xFF000000, 
             textStr1);
}


//------------------------------------------------------------------------
void FxMemoryImage::UpdateMemoFrag(SqueezeOption sqOpt, int fixedSize)
{
    static UInt8 updDuty = 0;
    if (updDuty++ > UpdateDutyFactor)
    {
        CreateMemoFrag(sqOpt, fixedSize);
        updDuty = 0;
    }
}


//------------------------------------------------------------------------
void FxMemoryImage::drawChartBar(unsigned idx, UPInt used, UPInt total, UInt32 color)
{
    UPInt mask      = (UPInt(1) << SizeShift) - 1;
    unsigned  endUsed   = (unsigned)((used + mask) >> SizeShift);
    unsigned  endUnused =  (unsigned)(total >> SizeShift);
    unsigned  x, y;
    UInt32 gradient = 64;
    UInt32 dgY = gradient / BarHeight;

    if (endUsed > Image.Width)
        endUsed = Image.Width;

    if (endUnused > Image.Width)
        endUnused = Image.Width;

    for(y = 0; y < BarHeight - 1; ++y)
    {
        UInt32* sl = (UInt32 *)Image.GetScanline(idx * BarHeight + y);
        UInt32 r = y * dgY + ((color >> 16) & 0xFF); if (r > 0xFF) r = 0xFF;
        UInt32 g = y * dgY + ((color >> 8 ) & 0xFF); if (g > 0xFF) g = 0xFF;
        UInt32 b = y * dgY + ((color      ) & 0xFF); if (b > 0xFF) b = 0xFF;

#if defined(SF_OS_PS3) 

        UInt32 tmpColor = 0xFF000000 | (r << 16) | (g << 8) | b;
        UInt32 c        = (r << 24) | (g << 16) | (b << 8) | 0xFF;

#elif defined(SF_OS_WII)

        UInt32 tmpColor = 0xFF000000 | (r << 16) | (g << 8) | b;
		UInt32 c        = (r << 24) | (g << 16) | (b << 8) | 0xFF;

#elif defined(SF_OS_LINUX)

        UInt32 tmpColor = 0xFF000000 | (r << 16) | (g << 8) | b;

        {
            UInt32 tmp = r;
            r = b;
            b = tmp;
        }
        
        UInt32 c = 0xFF000000 | (r << 16) | (g << 8) | b;
        
#else

        UInt32 c = 0xFF000000 | (r << 16) | (g << 8) | b;

#endif

        for(x = 0; x < endUsed; ++x)
        {
            sl[x] = c;
        }

        if (endUsed)
        {
#if defined(SF_OS_PS3) || defined(SF_OS_WII)

			sl[endUsed - 1] = ((((c >> 9) & 0x7F7F7F)) << 9) | 0xFF;
#else
            sl[endUsed - 1] = ((c >> 1) & 0xFF7F7F7F) | 0xFF000000;
#endif
      
        }
        
#if defined(SF_OS_PS3) || defined(SF_OS_LINUX) || defined(SF_OS_WII)

        c = tmpColor;
#endif

        r = gradient + ((c >> 16) & 0xFF); if (r > 0xFF) r = 0xFF;
        g = gradient + ((c >> 8 ) & 0xFF); if (g > 0xFF) g = 0xFF;
        b = gradient + ((c      ) & 0xFF); if (b > 0xFF) b = 0xFF;

#if defined(SF_OS_PS3)

       	c = (r << 24) | (g << 16) | (b << 8) | 0xFF;

#elif defined(SF_OS_WII)

       	c = (r << 24) | (g << 16) | (b << 8) | 0xFF;

#elif defined(SF_OS_LINUX)

        {
            UInt32 tmp = r;
            r = b;
            b = tmp;
        }

        c = 0xFF000000 | (r << 16) | (g << 8) | b;

#else

        c = 0xFF000000 | (r << 16) | (g << 8) | b;

#endif

        for(x = endUsed; x < endUnused; ++x)
        {
            sl[x] = c;
        }
    }
}


//------------------------------------------------------------------------
void FxMemoryImage::clearChartBar(unsigned idx, UPInt total)
{   
    for(unsigned y = 0; y < BarHeight - 1; ++y)
    {
        UInt32* sl = (UInt32 *)Image.GetScanline(idx * BarHeight + y);
        for(unsigned x = 0; x < (unsigned)total; ++x)
        {
            sl[x] = Background;
        }
    }
}


//------------------------------------------------------------------------
void FxMemoryImage::acquireHeaps(UPInt upperBound)
{
    Heaps.ClearAndRelease();
    MemoryHeap::RootStats rs;
    MemoryHeap::GetRootStats(&rs);
    FxMemHeap h1 = { 0, 0, 0, 0 };

    visitHeaps(Memory::pGlobalHeap);
    Alg::QuickSort(Heaps, cmpType);

    h1.Type = GHeapSegVisitor::Seg_SysMem;
    h1.Footprint = rs.SysMemFootprint;
    h1.UsedSpace = rs.SysMemUsedSpace;
    Heaps.PushBack(h1);

    h1.Type = GHeapSegVisitor::Seg_DebugInfo;
    h1.Footprint = rs.DebugInfoFootprint;
    h1.UsedSpace = rs.DebugInfoUsedSpace;
    Heaps.PushBack(h1);

    h1.Type = GHeapSegVisitor::Seg_Bookkeeper;
    h1.Footprint = rs.BookkeepingFootprint + rs.PageMapFootprint;
    h1.UsedSpace = rs.BookkeepingUsedSpace + rs.PageMapUsedSpace;
    Heaps.PushBack(h1);

    //h1.Type = GHeapSegVisitor::Seg_PageMap;
    //h1.Footprint = rs.PageMapFootprint;
    //h1.UsedSpace = rs.PageMapUsedSpace;
    //Heaps.PushBack(h1);

    NumChartBars = 0;
    maxTotal = 0;
    for (UPInt i = 0; i < Heaps.GetSize(); ++i)
    {
        NumChartBars++;
        const FxMemHeap& heap = Heaps[i];
        if (heap.Type == GHeapSegVisitor::Seg_SysMem)
        {
            // Compute max unused space for system memory
            if (maxTotal < heap.Footprint - heap.UsedSpace)
                maxTotal = heap.Footprint - heap.UsedSpace;
        }
        else
        {
            if (maxTotal < heap.Footprint)
                maxTotal = heap.Footprint;
        }
    }

    SizeShift = 0;
    while(maxTotal > upperBound)
    {
        maxTotal >>= 1;
        SizeShift++;
    }
}


//------------------------------------------------------------------------
void FxMemoryImage::CreateMemoChart()
{
    ClearImage();
    acquireHeaps(Image.Width);

    BarHeight = Image.Height / NumChartBars;
    UPInt barI = 0;
    for (UPInt i = 0; i < Heaps.GetSize(); ++i)
    {
        FxMemHeap heap = Heaps[i];

        UInt32 color = getColor(heap.Type, heap.Heap);

        if (heap.Footprint)
        {
            if (heap.Type == GHeapSegVisitor::Seg_SysMem)
            {
                // Show only unused for the system memory.
                heap.Footprint -= heap.UsedSpace;
                heap.UsedSpace  = 0;
            }

            drawChartBar((unsigned)barI, heap.UsedSpace, heap.Footprint, color);
            if (heap.Heap)
            {
                SF::StringBuffer barStr;

                Format(
                    barStr,
                    "{0}: Footprint = {1:sep:,}K UsedSpace = {2:sep:,}K",
                    heap.Heap->GetName(),
                    (heap.Footprint + 1023) / 1024,
                    (heap.UsedSpace + 1023) / 1024
                    );

                drawText((unsigned)(5), (unsigned)(barI * BarHeight + BarHeight - 6), 
                         0xFFFFFFFF, 0xFF000000, barStr);
            }
            // DBG
            //ChartHistory[i].Footprint = heap.Footprint;
            //ChartHistory[i].UsedSpace = heap.UsedSpace;
          
            barI++;
        }
    }
}


//------------------------------------------------------------------------
void FxMemoryImage::UpdateMemoChart(void)
{
    static unsigned updFactor = 0;

    acquireHeaps(Image.Width);

    BarHeight = Image.Height / NumChartBars;
    UPInt barI = 0;
    for (UPInt i = 0; i < Heaps.GetSize(); ++i)
    {
        FxMemHeap heap = Heaps[i]; 

        UInt32 color = getColor(heap.Type, heap.Heap);
        if (heap.Footprint)
        {
            if ((ChartHistory[i].Footprint != heap.Footprint) || 
                (ChartHistory[i].UsedSpace != heap.UsedSpace) ||
                (updFactor == UpdateDutyFactor))
            {
                if (heap.Type == GHeapSegVisitor::Seg_SysMem)
                {
                    // Show only unused for the system memory.
                    heap.Footprint -= heap.UsedSpace;
                    heap.UsedSpace  = 0;
                }
                clearChartBar((unsigned)barI, Image.Width);
                drawChartBar((unsigned)barI, heap.UsedSpace, heap.Footprint, color);
                if (heap.Heap)
                {
                    SF::StringBuffer barStr;

                    Format(
                        barStr,
                        "{0}: Footprint = {1:sep:,}K UsedSpace = {2:sep:,}K",
                        heap.Heap->GetName(),
                        (heap.Footprint + 1023) / 1024,
                        (heap.UsedSpace + 1023) / 1024
                        );

                    drawText((unsigned)(5), (unsigned)(barI * BarHeight + BarHeight - 6), 
                             0xFFFFFFFF, 0xFF000000, barStr);
                }
                // DBG
                //ChartHistory[i].Footprint = heap.Footprint;
                //ChartHistory[i].UsedSpace = heap.UsedSpace;
            }     
            ++barI;
        }
    }

    ++updFactor;

    if (updFactor > UpdateDutyFactor)
    {
        updFactor = 0;
    }
}


//------------------------------------------------------------------------
void FxMemoryImage::initChart(const FxMemHeap& heap, UPInt idx)
{
    GraphCharts[idx].Heap = heap.Heap;
    GraphCharts[idx].Type = heap.Type;
    GraphCharts[idx].Start = LastStart;
    GraphCharts[idx].MaxVal = 0;
    GraphCharts[idx].Color = getColor(heap.Type, heap.Heap);
    LastStart += Image.Width;
    memset(GraphCharts[idx].Start, 0, sizeof(FxMemHistory) * Image.Width);
}


//------------------------------------------------------------------------
void FxMemoryImage::clearLast()
{
    for (UPInt i = 0; i < NumCharts; ++i)
    {
        GraphCharts[i].MaxVal = 0;
        GraphCharts[i].Start[HistoryIdx].Footprint = 0;
        GraphCharts[i].Start[HistoryIdx].UsedSpace = 0;
    }
}


//------------------------------------------------------------------------
int FxMemoryImage::addToHistory(const FxMemHeap& heap, UPInt idx)
{
    if (idx >= NumCharts)
    {
        if (NumCharts >= MaxCharts)
        {
            return 0;
        }
        ++NumCharts;
        initChart(heap, idx);
    }
    else
    {
        if (heap.Type < GraphCharts[idx].Type)
        {
            if (NumCharts >= MaxCharts)
            {
                return 0;
            }
            memmove(GraphCharts + idx + 1, 
                    GraphCharts + idx,
                    sizeof(FxGraphChart) * (NumCharts - idx));
            NumCharts++;
            initChart(heap, idx);
        }
    }

    FxGraphChart& chart = GraphCharts[idx];

    UPInt prev = HistoryIdx ? HistoryIdx - 1 : 0;
    
    int changed = 
        chart.Start[prev].Footprint != heap.Footprint || 
        chart.Start[prev].UsedSpace != heap.UsedSpace;

    chart.Start[HistoryIdx].Footprint = heap.Footprint;
    chart.Start[HistoryIdx].UsedSpace = heap.UsedSpace;

    if (chart.MaxVal < heap.Footprint)
        chart.MaxVal = heap.Footprint;

    return changed;
}


static UInt32 alphaBlend(UInt32 dst, UInt32 src, UByte alpha)
{
    unsigned sr = (src >> 16) & 0xFF;
    unsigned sg = (src >> 8 ) & 0xFF;
    unsigned sb = (src      ) & 0xFF;
    unsigned dr = (dst >> 16) & 0xFF;
    unsigned dg = (dst >> 8 ) & 0xFF;
    unsigned db = (dst      ) & 0xFF;
    
    unsigned r = (((sr - dr) * alpha) + (dr << 8)) >> 8;
    unsigned g = (((sg - dg) * alpha) + (dg << 8)) >> 8;
    unsigned b = (((sb - db) * alpha) + (db << 8)) >> 8;

#if defined(SF_OS_LINUX)

    {
        UInt32 tmp = r;
        r = b;
        b = tmp;
    }

    UInt32 c = 0xFF000000 | (r << 16) | (g << 8) | b;

#elif defined(SF_OS_PS3)

    UInt32 c = (r << 24) | (g << 16) | (b << 8) | 0xFF;
    
#elif defined(SF_OS_WII)

	UInt32 c = (r << 24) | (g << 16) | (b << 8) | 0xFF;
	
#else

	UInt32 c = 0xFF000000 | (r << 16) | (g << 8) | b;

#endif

    return c;
}


//------------------------------------------------------------------------
void FxMemoryImage::drawGraphChart(const FxGraphChart& chart)
{
    UPInt x;
    for (x = 0; x <= HistoryIdx; ++x)
    {
        UPInt prevUsed = x ? chart.Start[x - 1].UsedSpace : 0;
        UPInt thisUsed = chart.Start[x].UsedSpace;

        prevUsed >>= SizeShift;
        thisUsed >>= SizeShift;
        if (prevUsed > thisUsed)
        {
            UPInt t = prevUsed; prevUsed = thisUsed; thisUsed = t;
        }

        if (prevUsed >= 3)
        {
            prevUsed -= 3;
        }
        else
        {
            thisUsed += 3 - prevUsed;
            prevUsed  = 0;
        }

        UInt32* sl;
        UPInt y;

        if (thisUsed > Image.Height - 1)
            thisUsed = Image.Height - 1;

        // DBG
        //// Alpha-blend from bottom to UsedSpace (may be too expensive)
        //sl = (UInt32*)Image.GetScanline(Image.Height - 1);
        //for (y = 0; y < prevUsed; ++y)
        //{
        //    sl[x] = alphaBlend(sl[x], chart.Color, 24);
        //    sl -= Image.Width;
        //}

        // Draw UsedSpace
        sl = (UInt32 *)Image.GetScanline((unsigned)(Image.Height - prevUsed - 1));
        for (y = prevUsed; y < thisUsed; ++y)
        {
            sl[x] = chart.Color;
            sl -= Image.Width;
        }

        UPInt prevFoot = x ? chart.Start[x-1].Footprint : 0;
        UPInt thisFoot = chart.Start[x].Footprint;
        prevFoot >>= SizeShift;
        thisFoot >>= SizeShift;

        if (prevFoot > Image.Height - 1)
            prevFoot = Image.Height - 1;

        if (thisFoot > Image.Height - 1)
            thisFoot = Image.Height - 1;

        // Alpha-blend from UsedSpace to Footprint (may be too expensive)
        sl = (UInt32 *)Image.GetScanline((unsigned)(Image.Height - thisUsed - 1));
        for (y = thisUsed; y < thisFoot; ++y)
        {
            sl[x] = alphaBlend(sl[x], chart.Color, 190);
            sl -= Image.Width;
        }

        if (prevFoot > thisFoot)
        {
            UPInt t = prevFoot; prevFoot = thisFoot; thisFoot = t;
        }

        // Draw Footprint
        sl = (UInt32 *)Image.GetScanline((unsigned)(Image.Height - prevFoot - 1));
        for (y = prevFoot; y <= thisFoot; ++y)
        {

#if defined(SF_OS_LINUX)

            {
                UInt32 c = chart.Color;
                UInt32 r = c & 0xFF;
                UInt32 g = (c >> 8) & 0xFF;
                UInt32 b = (c >> 16) & 0xFF;
                c = 0xFF000000 | (r << 16) | (g << 8) | b;
                sl[x] = c;
            }

#elif defined(SF_OS_PS3) 

            sl[x] = (chart.Color << 8) | 0xFF;

#elif defined(SF_OS_WII) 

            sl[x] = (chart.Color << 8) | 0xFF;

#else

            sl[x] = 0xFF000000 | chart.Color;

#endif
            sl -= Image.Width;
        }
    }
}


//------------------------------------------------------------------------
void FxMemoryImage::drawGraphChart(const FxGraphChart& chart, UPInt x, UPInt startY, UPInt h)
{
    UPInt prevUsed = x ? chart.Start[x - 1].UsedSpace : 0;
    UPInt thisUsed = chart.Start[x].UsedSpace;

    prevUsed >>= SizeShift;
    thisUsed >>= SizeShift;

    if (prevUsed > thisUsed)
    {
        UPInt t = prevUsed; prevUsed = thisUsed; thisUsed = t;
    }

    if (prevUsed >= h)
    {
        prevUsed -= h;
    }
    else
    {
        thisUsed += h - prevUsed;
        prevUsed  = 0;
    }

    UInt32* sl;
    UPInt y1, y2;

    y1 = startY;
    y2 = y1 + prevUsed;
    if (y1 > Image.Height - 1) y1 = Image.Height - 1;
    if (y2 > Image.Height - 1) y2 = Image.Height - 1;

    // Alpha-blend from bottom to UsedSpace (may be too expensive)
    sl = (UInt32 *)Image.GetScanline((unsigned)(Image.Height - 1 - y1));
    for (; y1 < y2; ++y1)
    {
        sl[x] = alphaBlend(sl[x], chart.Color, 128);
        sl -= Image.Width;
    }

    // Draw UsedSpace
    y1 = startY + prevUsed;
    y2 = startY + thisUsed;
    if (y1 > Image.Height-1) y1 = Image.Height-1;
    if (y2 > Image.Height-1) y2 = Image.Height-1;

    sl = (UInt32*)Image.GetScanline((unsigned)(Image.Height - 1 - y1));
    for (; y1 < y2; ++y1)
    {
       
#if defined(SF_OS_LINUX)

        {
            UInt32 c = chart.Color;
            UInt32 r = c & 0xFF;
            UInt32 g = (c >> 8) & 0xFF;
            UInt32 b = (c >> 16) & 0xFF;
            c = 0xFF000000 | (r << 16) | (g << 8) | b;
            sl[x] = c;
        }

#elif defined(SF_OS_PS3) 
         
        sl[x] = (chart.Color << 8) | 0xFF;

#elif defined(SF_OS_WII) 

        sl[x] = (chart.Color << 8) | 0xFF;

#else

        sl[x] = 0xFF000000 | chart.Color;

#endif

        sl -= Image.Width;
    }

    UPInt prevFoot = x ? chart.Start[x-1].Footprint : 0;
    UPInt thisFoot = chart.Start[x].Footprint;
    prevFoot >>= SizeShift;
    thisFoot >>= SizeShift;

    // Alpha-blend from UsedSpace to Footprint (may be too expensive)
    y1 = startY + thisUsed;
    y2 = startY + thisFoot;
    if (y1 > Image.Height-1) y1 = Image.Height-1;
    if (y2 > Image.Height-1) y2 = Image.Height-1;

    sl = (UInt32*)Image.GetScanline((unsigned)(Image.Height - 1 - y1));
    for (; y1 < y2; ++y1)
    {
        sl[x] = alphaBlend(sl[x], chart.Color, 190);
        sl -= Image.Width;
    }

    if (prevFoot > thisFoot)
    {
        UPInt t = prevFoot; prevFoot = thisFoot; thisFoot = t;
    }

    // Draw Footprint
    y1 = startY + prevFoot;
    y2 = startY + thisFoot;
    if (y1 > Image.Height-1) y1 = Image.Height-1;
    if (y2 > Image.Height-1) y2 = Image.Height-1;

    sl = (UInt32 *)Image.GetScanline((unsigned)(Image.Height - 1 - y1));
    for (; y1 <= y2; ++y1) // Yes, "<=" is correct here
    {
        sl[x] = chart.Color;
        sl -= Image.Width;
    }
}


//------------------------------------------------------------------------
void FxMemoryImage::DrawHorizLine(UInt32 xStart, UInt32 y, UInt32 xEnd, UInt32 color)
{
    UInt32* sl = (UInt32*)Image.GetScanline((unsigned)(y));
    for (UInt32 x = xStart; x < xEnd; ++x)
    {
        sl[x] = color;
    }
}


//------------------------------------------------------------------------
SF_INLINE void FxMemoryImage::drawVertLine(UInt32 x, UInt32 yStart, UInt32 yEnd, UInt32 color)
{
    for (UInt32 y = yStart; y < yEnd; ++y)
    {
        ((UInt32 *)Image.GetScanline((unsigned)(y)))[x] = color;
    }
}


//------------------------------------------------------------------------
SF_INLINE UInt32 FxMemoryImage::getBackgroundColor(UInt32 x, UInt32 y)
{
    return ((UInt32 *)Image.GetScanline((unsigned)(y)))[x]; 
}


float FxMemoryImage::GetTotMem(void)
{
    return totalMemory;
}


//------------------------------------------------------------------------
void FxMemoryImage::CreateMemoGram()
{
    ClearImage();
    //clearLast(); // DBG
    acquireHeaps(Image.Height);

    if (MemHistory == 0)
    {
        MemHistory = LastStart =
             (FxMemHistory*)MemoryHeap::AllocSysDirect(Image.Width * 
                                                        MaxCharts * 
                                                        sizeof(FxMemHistory));
        if (MemHistory == 0)
        {
            return;
        }
    }

    UPInt idx = 0;
    int changed = 0;
    for (SInt32 i = SInt32(Heaps.GetSize() - 1); i >= 0; --i)
    {
        FxMemHeap heap = Heaps[i];

        if (heap.Footprint)
        {
            if (heap.Type == GHeapSegVisitor::Seg_SysMem)
            {
                // Show only unused for the system memory.
                heap.Footprint -= heap.UsedSpace;
                heap.UsedSpace  = 0;
            }
            changed += addToHistory(heap, idx);
            idx++;
        }
    }

    UPInt maxGlobal = 0;

    SF_ASSERT(NumCharts <= MaxCharts);

    for (unsigned i = 0; i < NumCharts; ++i)
    {
        UPInt maxInChart = 0;
        for (UPInt j = 0; j < Image.Width; ++j)
        {
            if (maxInChart < GraphCharts[i].Start[j].Footprint)
                maxInChart = GraphCharts[i].Start[j].Footprint;
        }
        GraphCharts[i].MaxVal = maxInChart;
        maxGlobal += GraphCharts[i].MaxVal;
    }


    UPInt maxChart = maxGlobal;

    SizeShift = 0;
    while(maxGlobal > Image.Height)
    {
        maxGlobal >>= 1;
        SizeShift++;
    }

    totalMemory = (float)maxChart * (float)Image.Height / (float)maxGlobal;

    // For compatibility with MemoGram1.
    for (unsigned i = 0; i < NumCharts; ++i)
    {
        SortedChart[i] = GraphCharts[i];
    }

    if (SizeShift == PrevShift)
    {    
        for (unsigned i = 0; i < Image.Height; ++i)
        {
            UInt32* pix = (UInt32 *)Image.GetScanline(i);
            pix[HistoryIdx] = Background;
        }
    }
    PrevShift = SizeShift;

    for (UPInt x = 0; x <= HistoryIdx; ++x)
    {
        UPInt y = 0;
        for (unsigned i = 0; i < NumCharts; ++i)
        {
            drawGraphChart(SortedChart[i], x, y, 2);
            y += GraphCharts[i].Start[x].Footprint >> SizeShift;
        }
    }

    if (HistoryIdx < Image.Width - 1)
    {
        HistoryIdx++;
    }
    else
    {
        for (unsigned i = 0; i < NumCharts; ++i)
        {
            memmove(GraphCharts[i].Start, GraphCharts[i].Start + 1, 
                    sizeof(FxMemHistory) * (Image.Width - 1));
        }
        for (unsigned i = 0; i < Image.Height; ++i)
        {
            memmove(Image.GetScanline(i), Image.GetScanline(i) + 4, 
                    (UPInt)(Image.Width * 4 - 4));
        }
    }
}


//------------------------------------------------------------------------
void FxMemoryImage::UpdateMemoGram()
{
    // DBG
    //clearLast();
    acquireHeaps(Image.Height);

    UPInt idx = 0;
    int changed = 0;
    for (SInt32 i = SInt32(Heaps.GetSize() - 1); i >= 0 ; --i)
    {
        FxMemHeap heap = Heaps[i];

        if (heap.Footprint)
        {
            if (heap.Type == GHeapSegVisitor::Seg_SysMem)
            {
                // Show only unused for the system memory.
                heap.Footprint -= heap.UsedSpace;
                heap.UsedSpace  = 0;
            }
            changed += addToHistory(heap, idx);
            idx++;
        }
    }

    if (changed)
    {
        SteadyCount = 0;
    }
    else
    {
        if (++SteadyCount < 10)
        {
            changed = 1;
        }
    }

    if (!changed)
    {
        return;
    }

    UPInt maxGlobal = 0;

    for (unsigned i = 0; i < NumCharts; ++i)
    {
        UPInt maxInChart = 0;
        for (UPInt j = 0; j < Image.Width; ++j)
        {
            if (maxInChart < GraphCharts[i].Start[j].Footprint)
                maxInChart = GraphCharts[i].Start[j].Footprint;
        }
        GraphCharts[i].MaxVal = maxInChart;
        maxGlobal += GraphCharts[i].MaxVal;
    }


    UPInt maxChart = maxGlobal;

    SizeShift = 0;
    while(maxGlobal > Image.Height)
    {
        maxGlobal >>= 1;
        SizeShift++;
    }

    totalMemory = (float)maxChart * (float)Image.Height / (float)maxGlobal;

    // For compatibility with MemoGram1.
    for (unsigned i = 0; i < NumCharts; ++i)
    {
        SortedChart[i] = GraphCharts[i];
    }

    if (SizeShift == PrevShift)
    {
        for (unsigned i = 0; i < Image.Height; ++i)
        {
            UInt32* pix = (UInt32 *)Image.GetScanline(i);
            pix[HistoryIdx] = Background;
        }
    }

    if (PrevShift != SizeShift)
    {
        ClearImage();

        for (UPInt x = 0; x <= HistoryIdx; ++x)
        {
            UPInt y = 0;
            for (unsigned i = 0; i < NumCharts; ++i)
            {
                drawGraphChart(SortedChart[i], x, y, 2);
                y += GraphCharts[i].Start[x].Footprint >> SizeShift;
            }
        }
        PrevShift = SizeShift;
    }
    else
    {
        UPInt y = 0;
        for (unsigned i = 0; i < NumCharts; ++i)
        {
            drawGraphChart(SortedChart[i], HistoryIdx, y, 2);
            y += GraphCharts[i].Start[HistoryIdx].Footprint >> SizeShift;
        }

        if (HistoryIdx < Image.Width - 1)
        {
            HistoryIdx++;
        }
        else
        {
            for (unsigned i = 0; i < NumCharts; ++i)
            {
                memmove(GraphCharts[i].Start, GraphCharts[i].Start + 1, 
                        sizeof(FxMemHistory) * (Image.Width - 1));
            }
            for (unsigned i = 0; i < Image.Height; ++i)
            {
                memmove(Image.GetScanline(i), Image.GetScanline(i) + 4, 
                        (UPInt)(Image.Width * 4 - 4));
            }
        }
    }
}


//------------------------------------------------------------------------
UPInt FxMemoryImage::GetTotalAddrSpace() const
{ 
    return (Image.Width  / GetRectWidth()) * 
           (Image.Height / GetRectHeight()) * GetPageSize();
}

//------------------------------------------------------------------------
UPInt FxMemoryImage::GetRowAddrSpace() const
{
    return (Image.Width / GetRectWidth()) * GetPageSize();
}


//------------------------------------------------------------------------
void FxMemoryImage::visitSegments(MemoryHeap* root, GHeapSegVisitor* visitor)
{
    root->VisitHeapSegments(visitor);

    struct HolderVisitor : MemoryHeap::HeapVisitor
    {
        FxChildHeapArray<Ptr<MemoryHeap>, 32> Heaps;
        virtual void Visit(MemoryHeap*, MemoryHeap *heap)
        { 
            Heaps.PushBack(heap); 
        }
    };

    HolderVisitor hv;
    root->VisitChildHeaps(&hv);

    for (UPInt i = 0; i < hv.Heaps.GetSize(); ++i)
    {
        visitSegments(hv.Heaps[i], visitor);
    }
}


//------------------------------------------------------------------------
void FxMemoryImage::visitHeaps(MemoryHeap* root)
{
    if ((root->GetFlags() & MemoryHeap::Heap_UserDebug) == 0)
    {
        // Consider only "payload" heaps. Ignore the UserDebug ones.
        FxMemHeap h1;
        h1.Heap = root;
        h1.Type = root->GetId() << 8;
        if (h1.Type == 0) h1.Type = ~UPInt(0);
        h1.Footprint = root->GetFootprint();
        h1.UsedSpace = root->GetUsedSpace();
        Heaps.PushBack(h1);
    }

    struct HolderVisitor : MemoryHeap::HeapVisitor
    {
        FxChildHeapArray<Ptr<MemoryHeap>, 32> Heaps;
        virtual void Visit(MemoryHeap*, MemoryHeap *heap)
        { 
            Heaps.PushBack(heap); 
        }
    };

    HolderVisitor hv;
    root->VisitChildHeaps(&hv);

    for (UPInt i = 0; i < hv.Heaps.GetSize(); ++i)
    {
        visitHeaps(hv.Heaps[i]);
    }
}


//------------------------------------------------------------------------
void FxMemSegVisitor::Visit(unsigned cat, const MemoryHeap* heap, 
                            UPInt addr, UPInt size)
{
    Image->addSegment(cat, heap, addr, size);
}



//------------------------------------------------------------------------
void FxGlyphCacheVisitor::Visit(const GFxGlyphParam&, 
                                const RectF& uvRect, 
                                const R30::Texture* tex)
{
    Image->addCachedGlyph(tex, uvRect);
}


//------------------------------------------------------------------------
void FxMemoryImage::addCachedGlyph(const R30::Texture* tex, const RectF& uvRect)
{
    FxCachedGlyph glyph;
    glyph.Texture   = tex;
    glyph.GlyphRect = uvRect;
    CachedGlyphs.PushBack(glyph);
}


//------------------------------------------------------------------------
bool FxMemoryImage::DrawGlyphCacheTexture(R30::Renderer* ren, FontCacheManager* fc, unsigned idx, 
                                          unsigned x, unsigned y, unsigned w, unsigned h)
{
    CachedGlyphs.Clear();
    FxGlyphCacheVisitor visitor(this);

    fc->VisitGlyphs(&visitor);
    Alg::QuickSort(CachedGlyphs, cmpTexture);


    UPInt i;
    const R30::Texture* prevTex = 0;
    UPInt prevIdx = 0;
    UPInt startGlyph = 0;
    UPInt numRects = 0;
    for (i = 0; i < CachedGlyphs.GetSize(); ++i)
    {
        if (CachedGlyphs[i].Texture != prevTex)
        {
            ++prevIdx;
            prevTex = CachedGlyphs[i].Texture;
        }
        if (prevIdx >= idx+1)
        {
            startGlyph = i;
            UPInt j;
            for (j = i; j < CachedGlyphs.GetSize() && 
                        prevTex == CachedGlyphs[j].Texture; ++j)
            {
                ++numRects;
            }
            break;
        }
    }

    float usedArea = 0;

    if (numRects)
    {
        Array<R30::Renderer::BitmapDesc> rects;
        rects.Resize(numRects);
        FontCacheManager::TextureConfig conf;
        fc->GetTextureConfig(&conf);

        float kx = float(w) / float(conf.TextureWidth);
        float ky = float(h) / float(conf.TextureHeight);
        float ku = 1.0f / float(conf.TextureWidth);
        float kv = 1.0f / float(conf.TextureHeight);

        for (i = 0; i < numRects; ++i)
        {
            R30::Renderer::BitmapDesc* rect = &rects[i];
            const FxCachedGlyph&  glyph = CachedGlyphs[startGlyph + i];

            usedArea += glyph.GlyphRect.Width() * glyph.GlyphRect.Height();

            rect->Color                 = 0xFF000000;
            rect->Coords.x1           = x + glyph.GlyphRect.x1   * kx;
            rect->Coords.y1            = y + glyph.GlyphRect.y1    * ky;
            rect->Coords.x2          = x + glyph.GlyphRect.x2  * kx;
            rect->Coords.y2         = y + glyph.GlyphRect.y2 * ky;
            rect->TextureCoords.x1    = (glyph.GlyphRect.x1   + 1) * ku;
            rect->TextureCoords.y1     = (glyph.GlyphRect.y1    + 1) * kv;
            rect->TextureCoords.x2   = (glyph.GlyphRect.x2  - 1) * ku;
            rect->TextureCoords.y2  = (glyph.GlyphRect.y2 - 1) * kv;
        }
        Render::Matrix2D mat;
        ren->DrawBitmaps(&rects[0], (int)numRects, (int)0, (int)numRects, prevTex, mat);

        // DBG
        //float used = 100.0f * usedArea / (float(conf.TextureWidth) * float(conf.TextureHeight));
        //printf("Used: %.2f\n", used); // TO DO: Use
    }

    CachedGlyphs.Clear();
    return true;
}
