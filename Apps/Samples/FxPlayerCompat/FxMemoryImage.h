/**************************************************************************

Filename    :   FxMemoryImage.h
Content     :   Memory image to be displayed as a texture
                
Created     :   2009
Authors     :   Maxim Shemenarev, Boris Rayskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxMemoryImage_H
#define INC_FxMemoryImage_H

#include "Renderer30/R30_Image.h"
#include "Kernel/SF_MemoryHeap.h"
#include "GHeapTypes.h"
#include "Kernel/SF_Alg.h"
#include "GFx/GFx_FontCacheManager.h"


//------------------------------------------------------------------------
template<class T> class FxMemDataArray
{
    enum 
    { 
        MaxPages  = 32,
        PageShift = 12,
        PageSize  = 1 << PageShift, 
        PageMask  =      PageSize - 1
    };

public:
    typedef T ValueType;

    FxMemDataArray() : NumPages(0), Size(0) {}
    ~FxMemDataArray() { ClearAndRelease(); }

    void ClearAndRelease()
    {
        if(NumPages)
        {
            ValueType** page = Pages + NumPages - 1;
            UPInt freeCount = Size & PageMask;
            while(NumPages--)
            {
                MemoryHeap::FreeSysDirect(*page, PageSize * sizeof(ValueType));
                freeCount = PageSize;
                --page;
            }
        }
        Size = NumPages = 0;
    }

    void Clear()
    {
        Size = NumPages = 0;
    }

    void PushBack(const ValueType& v)
    {
        *acquireDataPtr() = v;
        ++Size;
    }

    UPInt GetSize() const 
    { 
        return Size; 
    }

    const ValueType& operator [] (UPInt i) const
    {
        return Pages[i >> PageShift][i & PageMask];
    }

    ValueType& operator [] (UPInt i)
    {
        return Pages[i >> PageShift][i & PageMask];
    }

private:
    SF_INLINE ValueType* acquireDataPtr()
    {
        UPInt np = Size >> PageShift;
        if(np >= NumPages)
        {
#ifdef SF_BUILD_DEBUG
            SF_ASSERT(np < MaxPages);
#else
            if (np >= MaxPages)
            {
                --Size;
                return Pages[Size >> PageShift] + (Size & PageMask);
            }
#endif
            Pages[np] = (ValueType*)MemoryHeap::AllocSysDirect(PageSize * sizeof(ValueType));
            NumPages++;
        }
        return Pages[np] + (Size & PageMask);
    }

    ValueType*  Pages[MaxPages];
    UPInt       NumPages;
    UPInt       Size;
};



//------------------------------------------------------------------------
class FxMemoryImage
{
    friend class FxMemSegVisitor;
    friend class FxGlyphCacheVisitor;

    enum { MaxCharts = 16 };


    //------------------------------------------------------------------------
    struct FxMemSegment
    {
        UPInt               Addr;
        UPInt               Size;
        const MemoryHeap*  Heap;
        UByte               Type;
        bool                NextSqueezed;
    };

    //------------------------------------------------------------------------
    struct FxMemHeap
    {
        const MemoryHeap*  Heap;
        UPInt               Type;
        UPInt               Footprint;
        UPInt               UsedSpace;
    };

    //------------------------------------------------------------------------
    struct FxMemHistory
    {
        UPInt               Footprint;
        UPInt               UsedSpace;
    };

    //------------------------------------------------------------------------
    struct FxGraphChart
    {
        UPInt               Type;
        const MemoryHeap*  Heap;
        FxMemHistory*       Start;
        UPInt               MaxVal;
        UInt32              Color;
    };

    //------------------------------------------------------------------------
    struct FxCachedGlyph
    {
        const R30::Texture*     Texture;
        RectF              GlyphRect;
    };

public:
    enum SqueezeOption
    {
        DontSqueeze,
        SqueezeAsNeeded,
        AlwaysSqueeze
    };

    FxMemoryImage();
    ~FxMemoryImage();

    bool            IsReady() const { return ImageBuffer != 0; }
    void            Create(UInt32 width, UInt32 height, UInt32 background);

    void            CreateMemoFrag(SqueezeOption sqOpt = SqueezeAsNeeded,
                                   int fixedSize = -1);
    void            UpdateMemoFrag(SqueezeOption sqOpt = SqueezeAsNeeded,
                                   int fixedSize = -1);

    void            CreateMemoChart(void);
    void            UpdateMemoChart(void);

    void            CreateMemoGram(void);
    void            UpdateMemoGram(void);

    void            ReleaseMemory(void);

    bool            DrawGlyphCacheTexture(R30::Renderer* ren, FontCacheManager* fc, unsigned idx, 
                                      unsigned x, unsigned y, unsigned w, unsigned h);

    R30::ImageBase*     GetImage()                  { return &Image;                    }
    UPInt           GetRowAddrSpace()   const;
    UPInt           GetTotalAddrSpace() const;
    UPInt           GetPageSize()       const   { return 4096;                      }
    unsigned            GetRectWidth()      const   { return WidthRamp[SizeRampIdx];    }
    unsigned            GetRectHeight()     const   { return HeightRamp[SizeRampIdx];   }

    UPInt           GetSizePerWidth()   const   { return UPInt(1) << SizeShift;     }
    unsigned            GetNumChartBars()   const   { return NumChartBars;              }
    unsigned            GetBarHeight()      const   { return BarHeight;                 }

    void            SetHeapColors(const UInt32* heapsColors, UInt32 numHeaps, 
                                  const UInt32* sysHeapsColors, UInt32 numSysHeaps);
    void            ClearImage();

    float           GetTotMem(void);
    void            DrawHorizLine(UInt32 xStart, UInt32 yStart, UInt32 xEnd, UInt32 color);

private:

    UPInt           calcSqueezeSize();
    bool            squeezeBiggestGap();
    UInt32          getColor(UPInt type, const MemoryHeap* heap) const;
    void            drawSegment(UPInt start, UPInt end, UInt32 color, bool unused);
    void            drawSqueezedRow(UPInt start);

    void            drawChartBar(unsigned idx, UPInt used, UPInt total, UInt32 color);
    void            clearChartBar(unsigned idx, UPInt total);

    void            addSegment(unsigned type, const MemoryHeap* heap, 
                           UPInt addr, UPInt size);

    void            addCachedGlyph(const R30::Texture* tex, const RectF& uvRect);

    void            visitSegments(MemoryHeap* root, GHeapSegVisitor* visitor);
    void            visitHeaps(MemoryHeap* root);

    void            acquireHeaps(UPInt upperBound);

    inline void     initChart(const FxMemHeap& heap, UPInt idx);
    inline void     clearLast();
    inline int      addToHistory(const FxMemHeap& heap, UPInt idx);
    inline void     drawGraphChart(const FxGraphChart& chart);
    inline void     drawGraphChart(const FxGraphChart& chart, UPInt x, UPInt startY, UPInt h);

    inline void     drawChar(unsigned x, unsigned y, UInt32 c, const UByte* bits);
    inline void     drawChar(unsigned x, unsigned y, UInt32 c1, UInt32 c2, unsigned code);
    inline void     drawText(unsigned x, unsigned y, UInt32 c1, UInt32 c2, const char* text);

   
    inline void     drawVertLine(UInt32 xStart, UInt32 yStart, UInt32 yEnd, UInt32 color);
    inline UInt32   getBackgroundColor(UInt32 x, UInt32 y);

    static bool cmpSegAddr(const FxMemSegment& a, const FxMemSegment& b)
    {
        if (a.Addr != b.Addr)
        {
            return a.Addr < b.Addr;
        }
        return (a.Heap != b.Heap) ? 
                a.Heap < b.Heap: 
                a.Type < b.Type;
    }

    static bool cmpType(const FxMemHeap& a, const FxMemHeap& b)
    {
        return a.Type < b.Type;
    }

    static bool cmpTexture(const FxCachedGlyph& a, const FxCachedGlyph& b)
    {
        return a.Texture < b.Texture;
    }

    static bool cmpFootprint(const FxGraphChart& a, const FxGraphChart& b)
    {
        return a.MaxVal > b.MaxVal;
    }

    UInt32                          Background;
    FxMemDataArray<FxMemSegment>    Segments;
    FxMemDataArray<FxMemHeap>       Heaps;
    Array<FxCachedGlyph>           CachedGlyphs;
    UByte*                          ImageBuffer;
    R30::ImageBase                      Image;
    unsigned                            SizeRampIdx;
    unsigned                            PrevRampIdx;
    unsigned                            NumChartBars;
    unsigned                            BarHeight;
    unsigned                            SizeShift;
    unsigned                            PrevShift;
    UPInt                           maxTotal;
    FxMemHistory*                   MemHistory;
    FxMemHistory*                   LastStart;
    UPInt                           HistoryIdx;
    FxGraphChart                    GraphCharts[MaxCharts];
    FxGraphChart                    SortedChart[MaxCharts];
    FxMemHistory                    ChartHistory[MaxCharts];
    UPInt                           NumCharts;
    UPInt                           SteadyCount;

    const static unsigned               WidthRamp[];
    const static unsigned               HeightRamp[];

    const UInt32*                   FxImportantHeapsColors;
    UInt32                          NumHeaps;

    const UInt32*                   FxSysHeapColors;
    UInt32                          NumSysHeaps;
    UInt8                           UpdateDutyFactor;
    float                           totalMemory;
};



//------------------------------------------------------------------------
template<class T, UPInt MaxEmbedded = 16> class FxChildHeapArray
{
public:
    typedef T ValueType;

    FxChildHeapArray() : NumEmbedded(0) {}

    UPInt GetSize() const 
    { 
        return NumEmbedded + Extended.GetSize(); 
    }

    void PushBack(const ValueType& v)
    {
        if (NumEmbedded < MaxEmbedded)
            Embedded[NumEmbedded++] = v;
        else
            Extended.PushBack(v);
    }

    const ValueType& operator [] (UPInt i) const
    {
        return i < MaxEmbedded ? Embedded[i] : Extended[i - MaxEmbedded];
    }

    ValueType& operator [] (UPInt i)
    {
        return i < MaxEmbedded ? Embedded[i] : Extended[i - MaxEmbedded];
    }

private:

    UPInt               NumEmbedded;
    ValueType           Embedded[MaxEmbedded];
    Array<ValueType>   Extended;
};



//------------------------------------------------------------------------
class FxMemSegVisitor : public GHeapSegVisitor
{
public:
    FxMemSegVisitor(FxMemoryImage* image) : Image(image) {}

    virtual void Visit(unsigned cat, const MemoryHeap* heap, 
                       UPInt addr, UPInt size);

private:
    FxMemoryImage* Image;
};


//------------------------------------------------------------------------
class FxGlyphCacheVisitor : public GlyphCacheVisitor
{
public:
    FxGlyphCacheVisitor (FxMemoryImage* image) : Image(image) {}

    virtual void Visit(const GlyphParam& param, 
                       const RectF& uvRect, 
                       const R30::Texture* tex);

private:

    FxMemoryImage* Image;
};


#endif

