/**************************************************************************

Filename    :   Render_FontProviderWinAPI.cpp
Content     :   Windows API Font provider (GetGlyphOutline)
Created     :   6/21/2007
Authors     :   Maxim Shemanarev

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

#include "Render_FontProviderWinAPI.h"
#include "Kernel/SF_Std.h"

#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"

#include <Strsafe.h>

namespace Scaleform { namespace Render {

//------------------------------------------------------------------------
struct GFxGdiSelectObjectGuard
{
    HDC     WinDC;
    HGDIOBJ WinObj;
    GFxGdiSelectObjectGuard(HDC dc, HGDIOBJ obj) : WinDC(dc), WinObj(::SelectObject(dc, obj)) {}
   ~GFxGdiSelectObjectGuard() { ::SelectObject(WinDC, WinObj); }
};

//------------------------------------------------------------------------
ExternalFontWinAPI::~ExternalFontWinAPI()
{
    if (HintedFont)
        ::DeleteObject(HintedFont);

    if (MasterFont)
        ::DeleteObject(MasterFont);
}

//------------------------------------------------------------------------
int CALLBACK EnumFontFamExProc(
                              ENUMLOGFONTEXW *lpelfe, // logical-font data
                              NEWTEXTMETRICEXW *lpntme, // physical-font data
                              DWORD FontType, // type of font
                              LPARAM lParam // application-defined data
                              )
{
    SF_UNUSED3(lpelfe,lpntme,FontType);
    bool* found = (bool*)lParam;
    *found = true;
    return 0;
}

//------------------------------------------------------------------------
ExternalFontWinAPI::ExternalFontWinAPI(FontProviderWinAPI *pprovider,
                                       FontSysDataWinAPI* sysData, 
                                       const char* name, 
                                       unsigned fontFlags,
                                       Lock* fontLock)
    : Font(fontFlags),
      pFontProvider(pprovider),
      pSysData(sysData),
      MasterFont(0), HintedFont(0), LastHintedFontSize(0),
      Scale1024(1024.0f / float(GGO_FontHeight)),
      pFontLock(fontLock)
      
{
    Name.Resize(SFstrlen(name) + 1);
    SFstrcpy(&Name[0], SFstrlen(name) + 1, name);
   
    // Font name is encoded as UTF8, so unpack.
    UPInt bufLen = UTF8Util::GetLength(name) + 1;
    NameW.Resize(bufLen);
    UTF8Util::DecodeStringSafe(&NameW[0], bufLen, name);

    LOGFONTW lf;
    if (::StringCchCopyW((LPWSTR)&lf.lfFaceName, sizeof(lf.lfFaceName)/sizeof(lf.lfFaceName[0]), &NameW[0]) != S_OK)
    {
        SF_DEBUG_WARNING2(1, "WinAPI font provider: Font name '%s' is too long, must be no longer than %d chars", 
            name, sizeof(lf.lfFaceName)/sizeof(lf.lfFaceName[0]));
    }
    lf.lfCharSet = DEFAULT_CHARSET;

    bool found = false;
    EnumFontFamiliesExW(pSysData->WinHDC, &lf, (FONTENUMPROCW)EnumFontFamExProc, (LPARAM)&found, 0);
    if (!found)
    {
        // do not abort if font name is _sans, _typewriter or _serif;
        // let system choose the font
        if (SFstrcmp(name, "_sans") != 0 && 
            SFstrcmp(name, "_typewriter") != 0 && 
            SFstrcmp(name, "_serif") != 0)
            return;
    }

    MasterFont = ::CreateFontW(-GGO_FontHeight,             // height of font
                                0,                          // average character width
                                0,                          // angle of escapement
                                0,                          // base-line orientation angle
                                IsBold() ? FW_BOLD : FW_NORMAL, // font weight
                                IsItalic(),                 // italic attribute option
                                0,                          // underline attribute option
                                0,                          // strikeout attribute option
                                DEFAULT_CHARSET,            // character set identifier
                                OUT_DEFAULT_PRECIS,         // output precision
                                CLIP_DEFAULT_PRECIS,        // clipping precision
                                ANTIALIASED_QUALITY,        // output quality
                                DEFAULT_PITCH,              // pitch and family
                                &NameW[0]);                 // typeface name

    if(MasterFont)
    {
        TEXTMETRICW tm;
        GFxGdiSelectObjectGuard g1(pSysData->WinHDC, MasterFont);
        if (::GetTextMetricsW(pSysData->WinHDC, &tm))
        {
            SetFontMetrics(float(tm.tmExternalLeading) * Scale1024, 
                float(tm.tmAscent) * Scale1024, 
                float(tm.tmDescent) * Scale1024);
            loadKerningPairs();
        }
        else
        {
            // GetTextMetrics may return FALSE sometimes; not sure what the best
            // strategy is in this case, just print an error in debug output.
            //SF_ASSERT(0);
            SF_DEBUG_WARNING1(1, "ExternalFontWinAPI: GetTextMetricsW for '%s' returned FALSE, text rendering may be affected.",
                name);
        }
    }
}

//------------------------------------------------------------------------
void ExternalFontWinAPI::SetHinting(const Font::NativeHintingType& pnh)
{
    Hinting.MaxRasterHintedSize = pnh.MaxRasterHintedSize;
    Hinting.MaxVectorHintedSize = pnh.MaxVectorHintedSize;
    Hinting.RasterRange         = pnh.RasterRange;
    Hinting.VectorRange         = pnh.VectorRange;
}

//------------------------------------------------------------------------
void ExternalFontWinAPI::loadKerningPairs()
{
    Array<KERNINGPAIR> pairs;
    unsigned size = ::GetKerningPairsW(pSysData->WinHDC, 0, 0);
    if(size)
    {
        pairs.Resize(size);
        ::GetKerningPairsW(pSysData->WinHDC, size, &pairs[0]);
    }
    KerningPairs.Clear();
    for(unsigned i = 0; i < pairs.GetSize(); ++i)
    {
        KerningPairType pair;
        pair.Char0 = pairs[i].wFirst;
        pair.Char1 = pairs[i].wSecond;
        KerningPairs.Add(pair, float(pairs[i].iKernAmount) * Scale1024);
    }
}

//------------------------------------------------------------------------
bool ExternalFontWinAPI::decomposeGlyphOutline(const UByte* data, unsigned size, GlyphShape* shape, unsigned hintedSize)
{
    const UByte* curGlyph = data;
    const UByte* endGlyph = data + size;
    bool  hinted = hintedSize != 0;
    ShapePosInfo pos(0);

    shape->Clear();
    while(curGlyph < endGlyph)
    {
        const TTPOLYGONHEADER* th = (TTPOLYGONHEADER*)curGlyph;
        
        const UByte* endPoly = curGlyph + th->cb;
        const UByte* curPoly = curGlyph + sizeof(TTPOLYGONHEADER);

        if (hinted)
            shape->StartPath(&pos, Shape_NewPath, 1, 0, 0, FxToTwips(th->pfxStart.x), -FxToTwips(th->pfxStart.y));
        else
            shape->StartPath(&pos, Shape_NewPath, 1, 0, 0, FxToS1024(th->pfxStart.x), -FxToS1024(th->pfxStart.y));

        while(curPoly < endPoly)
        {
            const TTPOLYCURVE* pc = (const TTPOLYCURVE*)curPoly;
            
            if (pc->wType == TT_PRIM_LINE)
            {
                int i;
                for (i = 0; i < pc->cpfx; i++)
                {
                    if (hinted)
                        shape->LineTo(&pos, FxToTwips(pc->apfx[i].x), -FxToTwips(pc->apfx[i].y));
                    else
                        shape->LineTo(&pos, FxToS1024(pc->apfx[i].x), -FxToS1024(pc->apfx[i].y));
                }
            }
            
            if (pc->wType == TT_PRIM_QSPLINE)
            {
                int u;
                for (u = 0; u < pc->cpfx - 1; u++)  // Walk through points in spline
                {
                    POINTFX pntB = pc->apfx[u];     // B is always the current point
                    POINTFX pntC = pc->apfx[u+1];
                    
                    if (u < pc->cpfx - 2)           // If not on last spline, compute C
                    {
                        // midpoint (x,y)
                        *(int*)&pntC.x = (*(int*)&pntB.x + *(int*)&pntC.x) / 2;
                        *(int*)&pntC.y = (*(int*)&pntB.y + *(int*)&pntC.y) / 2;
                    }
                    if (hinted)
                        shape->QuadTo(&pos, FxToTwips(pntB.x), -FxToTwips(pntB.y),
                                            FxToTwips(pntC.x), -FxToTwips(pntC.y));
                    else
                        shape->QuadTo(&pos, FxToS1024(pntB.x), -FxToS1024(pntB.y),
                                            FxToS1024(pntC.x), -FxToS1024(pntC.y));
                }
            }
            curPoly += sizeof(WORD) * 2 + sizeof(POINTFX) * pc->cpfx;
        }
        curGlyph += th->cb;
        shape->ClosePath(&pos);
        shape->EndPath();
    }
    if (!shape->IsEmpty())
    {
        shape->EndShape();
        shape->SetHintedSize(hinted ? hintedSize*20 : 0);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------
class GFxBitsetIterator
{
public:
    GFxBitsetIterator(const UByte* bits, unsigned offset = 0) :
        Bits(bits + (offset >> 3)),
        Mask(UByte(0x80 >> (offset & 7)))
    {}

    void operator ++ ()
    {
        Mask >>= 1;
        if(Mask == 0)
        {
            ++Bits;
            Mask = 0x80;
        }
    }

    unsigned GetBit() const
    {
        return (*Bits) & Mask;
    }

private:
    const UByte* Bits;
    UByte        Mask;
};

//------------------------------------------------------------------------
void ExternalFontWinAPI::decomposeGlyphBitmap(const UByte* data, 
                                              int w, int h, int x, int y, 
                                              GlyphRaster* raster)
{
    raster->Width  = w;
    raster->Height = h;
    raster->OriginX = -x;
    raster->OriginY =  y;
    raster->Raster.Resize(w * h);

    int i;
    int pitch = ((w + 31) >> 5) << 2;

    const UByte* src = data;
          UByte* dst = &raster->Raster[0];
    for(i = 0; i < h; i++)
    {
        GFxBitsetIterator bits(src, 0);
        int j;
        for(j = 0; j < w; j++)
        {
            *dst++ = bits.GetBit() ? 255 : 0;
            ++bits;
        }
        src += pitch;
    }
}

//------------------------------------------------------------------------
int ExternalFontWinAPI::GetGlyphIndex(UInt16 code)
{
    if (MasterFont)
    {
        Lock::Locker locker(pFontLock);
        const unsigned* indexPtr = CodeTable.Get(code);
        if (indexPtr)
            return *indexPtr;

        GFxGdiSelectObjectGuard g1(pSysData->WinHDC, MasterFont);
        GLYPHMETRICS gm;
        MAT2 im = { {0,1}, {0,0}, {0,0}, {0,1} };
        int ret = ::GetGlyphOutlineW(pSysData->WinHDC,
                                     code,
                                     GGO_METRICS,
                                     &gm,
                                     0, 0,
                                     &im);

        if (ret == GDI_ERROR)
            return -1;

        GlyphType glyph;
        glyph.Code      =  code;
        glyph.Advance   =  float(gm.gmCellIncX) * Scale1024;
        glyph.Bounds.x1 =  float(gm.gmptGlyphOrigin.x) * Scale1024;
        glyph.Bounds.y1 = -float(gm.gmptGlyphOrigin.y) * Scale1024;
        glyph.Bounds.x2 =  glyph.Bounds.x1 + float(gm.gmBlackBoxX) * Scale1024;
        glyph.Bounds.y2 =  glyph.Bounds.y1 + float(gm.gmBlackBoxY) * Scale1024;

        Glyphs.PushBack(glyph);
        CodeTable.Add(code, (unsigned)Glyphs.GetSize()-1);
        return (unsigned)Glyphs.GetSize()-1;
    }
    return -1;
}

//------------------------------------------------------------------------
bool ExternalFontWinAPI::IsHintedVectorGlyph(unsigned glyphIndex, unsigned hintedSize) const
{
    if (IsMissingGlyph(glyphIndex) ||
        Hinting.VectorRange == DontHint ||
        hintedSize > Hinting.MaxVectorHintedSize)
    {
        return false;
    }

    if (Hinting.VectorRange == HintAll)
        return true;

    Lock::Locker locker(pFontLock);
    return IsCJK(UInt16(Glyphs[glyphIndex].Code));
}

//------------------------------------------------------------------------
bool ExternalFontWinAPI::IsHintedRasterGlyph(unsigned glyphIndex, unsigned hintedSize) const
{
    if (IsMissingGlyph(glyphIndex) ||
        Hinting.RasterRange == DontHint ||
        hintedSize > Hinting.MaxRasterHintedSize)
    {
        return false;
    }

    if (Hinting.RasterRange == HintAll)
        return true;

    Lock::Locker locker(pFontLock);
    return IsCJK(UInt16(Glyphs[glyphIndex].Code));
}

//------------------------------------------------------------------------
bool ExternalFontWinAPI::GetTemporaryGlyphShape(unsigned glyphIndex, unsigned hintedSize,
                                                GlyphShape* shape)
{
    if (IsMissingGlyph(glyphIndex))
        return 0;

    if (!IsHintedVectorGlyph(glyphIndex, hintedSize))
        hintedSize = 0;

    Lock::Locker locker(pFontLock);
    GlyphType& glyph = Glyphs[glyphIndex];

    HFONT hFont = MasterFont;
    if (hintedSize)
    {
        if (hintedSize != LastHintedFontSize)
        {
            if (HintedFont)
                ::DeleteObject(HintedFont);

            HintedFont = ::CreateFontW(-int(hintedSize),            // height of font
                                        0,                          // average character width
                                        0,                          // angle of escapement
                                        0,                          // base-line orientation angle
                                        IsBold() ? FW_BOLD : FW_NORMAL, // font weight
                                        IsItalic(),                 // italic attribute option
                                        0,                          // underline attribute option
                                        0,                          // strikeout attribute option
                                        DEFAULT_CHARSET,            // character set identifier
                                        OUT_DEFAULT_PRECIS,         // output precision
                                        CLIP_DEFAULT_PRECIS,        // clipping precision
                                        ANTIALIASED_QUALITY,        // output quality
                                        DEFAULT_PITCH,              // pitch and family
                                        &NameW[0]);                 // typeface name
            LastHintedFontSize = hintedSize;
        }
        hFont = HintedFont;
    }

    GFxGdiSelectObjectGuard g1(pSysData->WinHDC, hFont);

    if (pSysData->GlyphBuffer.GetSize() == 0)
        pSysData->GlyphBuffer.Resize(FontSysDataWinAPI::BufSizeInc);

    #ifndef GGO_UNHINTED         // For compatibility with old SDKs.
    #define GGO_UNHINTED 0x0100
    #endif

    GLYPHMETRICS gm;
    int totalSize = 0;
    MAT2 im = { {0,1}, {0,0}, {0,0}, {0,1} };
    totalSize = ::GetGlyphOutlineW(pSysData->WinHDC,
                                   glyph.Code,
                                   GGO_NATIVE,
                                   &gm,
                                   (DWORD)pSysData->GlyphBuffer.GetSize(),
                                   &pSysData->GlyphBuffer[0],
                                   &im);

    if (totalSize == GDI_ERROR ||
        totalSize > (int)pSysData->GlyphBuffer.GetSize())
    {
        // An error has occurred. Most probably it's because
        // of not enough room in the buffer. So, request the buffer size,
        // reallocate, and try again. The second call may fail for
        // some other reason (no glyph); in this case there's nothing 
        // else to do, just return NULL. 
        // The buffer is reallocated with adding BufSizeInc in order
        // to reduce the number of reallocations.
        //----------------------------------
        totalSize = ::GetGlyphOutlineW(pSysData->WinHDC,
                                       glyph.Code,
                                       GGO_NATIVE,
                                       &gm, 0, 0, &im);

        if(totalSize == GDI_ERROR) return 0;

        pSysData->GlyphBuffer.Resize(totalSize + FontSysDataWinAPI::BufSizeInc);
        totalSize = ::GetGlyphOutlineW(pSysData->WinHDC,
                                       glyph.Code,
                                       GGO_NATIVE,
                                       &gm,
                                       (DWORD)pSysData->GlyphBuffer.GetSize(),
                                       &pSysData->GlyphBuffer[0],
                                       &im);
        if (totalSize == GDI_ERROR ||
            totalSize > (int)pSysData->GlyphBuffer.GetSize())
            return 0;
    }

    if (totalSize)
    {
        if (!decomposeGlyphOutline(&pSysData->GlyphBuffer[0], totalSize, shape, hintedSize))
            return false;
    }
    return true;
}

//------------------------------------------------------------------------
bool ExternalFontWinAPI::GetGlyphRaster(unsigned glyphIndex, unsigned hintedSize,
                                        GlyphRaster* raster)
{
    if (IsMissingGlyph(glyphIndex))
        return false;

    if (!IsHintedRasterGlyph(glyphIndex, hintedSize))
        return 0;

    Lock::Locker locker(pFontLock);
    GlyphType& glyph = Glyphs[glyphIndex];
    if (hintedSize != LastHintedFontSize)
    {
        if (HintedFont)
            ::DeleteObject(HintedFont);

        HintedFont = ::CreateFontW(-int(hintedSize),             // height of font
                                    0,                          // average character width
                                    0,                          // angle of escapement
                                    0,                          // base-line orientation angle
                                    IsBold() ? FW_BOLD : FW_NORMAL, // font weight
                                    IsItalic(),                 // italic attribute option
                                    0,                          // underline attribute option
                                    0,                          // strikeout attribute option
                                    DEFAULT_CHARSET,            // character set identifier
                                    OUT_DEFAULT_PRECIS,         // output precision
                                    CLIP_DEFAULT_PRECIS,        // clipping precision
                                    ANTIALIASED_QUALITY,        // output quality
                                    DEFAULT_PITCH,              // pitch and family
                                    &NameW[0]);                 // typeface name
        LastHintedFontSize = hintedSize;
    }

    GFxGdiSelectObjectGuard g1(pSysData->WinHDC, HintedFont);

    if (pSysData->GlyphBuffer.GetSize() == 0)
        pSysData->GlyphBuffer.Resize(FontSysDataWinAPI::BufSizeInc);

    GLYPHMETRICS gm;
    int totalSize = 0;
    MAT2 im = { {0,1}, {0,0}, {0,0}, {0,1} };
    totalSize = ::GetGlyphOutlineW(pSysData->WinHDC,
                                   glyph.Code,
                                   GGO_BITMAP,
                                   &gm,
                                   (DWORD)pSysData->GlyphBuffer.GetSize(),
                                   &pSysData->GlyphBuffer[0],
                                   &im);

    if (totalSize == GDI_ERROR ||
        totalSize > (int)pSysData->GlyphBuffer.GetSize())
    {
        // An error has occurred. Most probably it's because
        // of not enough room in the buffer. So, request the buffer size,
        // reallocate, and try again. The second call may fail for
        // some other reason (no glyph); in this case there's nothing 
        // else to do, just return NULL. 
        // The buffer is reallocated with adding BufSizeInc in order
        // to reduce the number of reallocations.
        //----------------------------------
        totalSize = ::GetGlyphOutlineW(pSysData->WinHDC,
                                       glyph.Code,
                                       GGO_BITMAP,
                                       &gm, 0, 0, &im);

        if(totalSize == GDI_ERROR) return 0;

        pSysData->GlyphBuffer.Resize(totalSize + FontSysDataWinAPI::BufSizeInc);
        totalSize = ::GetGlyphOutlineW(pSysData->WinHDC,
                                       glyph.Code,
                                       GGO_BITMAP,
                                       &gm,
                                       (DWORD)pSysData->GlyphBuffer.GetSize(),
                                       &pSysData->GlyphBuffer[0],
                                       &im);
        if (totalSize == GDI_ERROR ||
            totalSize > (int)pSysData->GlyphBuffer.GetSize())
            return 0;
    }

    if (totalSize)
    {
        decomposeGlyphBitmap(&pSysData->GlyphBuffer[0],
                             gm.gmBlackBoxX,
                             gm.gmBlackBoxY,
                             gm.gmptGlyphOrigin.x,
                             gm.gmptGlyphOrigin.y,
                             raster);
        return true;
    }
    else
    {
        raster->Width  = 1;
        raster->Height = 1;
        raster->Raster.Resize(1);
        raster->Raster[0] = 0;
    }
    return true;
}


//------------------------------------------------------------------------
float ExternalFontWinAPI::GetAdvance(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex))
        return GetNominalGlyphWidth();

    Lock::Locker locker(pFontLock);
    return Glyphs[glyphIndex].Advance;
}

//------------------------------------------------------------------------
float ExternalFontWinAPI::GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const
{
    float adjustment;
    KerningPairType k;
    k.Char0 = (UInt16)lastCode;
    k.Char1 = (UInt16)thisCode;
    if (KerningPairs.Get(k, &adjustment))
    {
        return adjustment;
    }
    return 0;
}

//------------------------------------------------------------------------
float ExternalFontWinAPI::GetGlyphWidth(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex))
        return GetNominalGlyphWidth();

    Lock::Locker locker(pFontLock);
    const RectF& r = Glyphs[glyphIndex].Bounds;
    return r.Width();
}

//------------------------------------------------------------------------
float ExternalFontWinAPI::GetGlyphHeight(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex))
        return GetNominalGlyphHeight();

    Lock::Locker locker(pFontLock);
    const RectF& r = Glyphs[glyphIndex].Bounds;
    return r.Height();
}

//------------------------------------------------------------------------
RectF& ExternalFontWinAPI::GetGlyphBounds(unsigned glyphIndex, RectF* prect) const
{
    if (IsMissingGlyph(glyphIndex))
        prect->SetRect(GetNominalGlyphWidth(), GetNominalGlyphHeight());
    else
    {
        Lock::Locker locker(pFontLock);
        *prect = Glyphs[glyphIndex].Bounds;
    }
    return *prect;
}

//------------------------------------------------------------------------
const char* ExternalFontWinAPI::GetName() const
{
    return &Name[0];
}







//------------------------------------------------------------------------
FontProviderWinAPI::FontProviderWinAPI(HDC dc): 
    SysData(dc)
{
    Font::NativeHintingType nhAllFonts;
    nhAllFonts.RasterRange = Font::HintCJK;
    nhAllFonts.VectorRange = Font::DontHint;
    nhAllFonts.MaxRasterHintedSize = 24;
    nhAllFonts.MaxVectorHintedSize = 24;
    NativeHinting.PushBack(nhAllFonts);
}

//------------------------------------------------------------------------
FontProviderWinAPI::~FontProviderWinAPI()
{    
}

//------------------------------------------------------------------------
Font::NativeHintingType* FontProviderWinAPI::findNativeHinting(const char* name)
{
    unsigned i;
    for (i = 0; i < NativeHinting.GetSize(); ++i)
    {
        if (NativeHinting[i].Typeface.CompareNoCase(name) == 0)
            return &NativeHinting[i];
    }
    return 0;
}

//------------------------------------------------------------------------
void FontProviderWinAPI::SetHinting(const char* name, 
                                    Font::NativeHintingRange vectorRange, 
                                    Font::NativeHintingRange rasterRange, 
                                    unsigned maxVectorHintedSize,
                                    unsigned maxRasterHintedSize)
{
    Font::NativeHintingType* pnh = findNativeHinting(name);
    if (pnh == 0)
    {
        Font::NativeHintingType nh = NativeHinting[0];
        nh.Typeface = name;
        NativeHinting.PushBack(nh);
        pnh = &NativeHinting[NativeHinting.GetSize()-1];
    }
    pnh->VectorRange         = vectorRange;
    pnh->RasterRange         = rasterRange;
    pnh->MaxVectorHintedSize = maxVectorHintedSize;
    pnh->MaxRasterHintedSize = maxRasterHintedSize;
}

//------------------------------------------------------------------------
void FontProviderWinAPI::SetHintingAllFonts(Font::NativeHintingRange vectorRange, 
                                            Font::NativeHintingRange rasterRange, 
                                            unsigned maxVectorHintedSize,
                                            unsigned maxRasterHintedSize)
{
    NativeHinting[0].VectorRange         = vectorRange;
    NativeHinting[0].RasterRange         = rasterRange;
    NativeHinting[0].MaxVectorHintedSize = maxVectorHintedSize;
    NativeHinting[0].MaxRasterHintedSize = maxRasterHintedSize;
}

//------------------------------------------------------------------------
Font* FontProviderWinAPI::CreateFont(const char* name, unsigned fontFlags)
{
//if (fontFlags & Font::FF_BoldItalic) return 0; // Faux bold & italic DBG

    // Mask flags to be safe.
    fontFlags &= Font::FF_CreateFont_Mask;
    fontFlags |= Font::FF_DeviceFont | Font::FF_NativeHinting;

    // Return a newly created font with a default RefCount of 1.
    // It is users responsibility to cache the font and release
    // it when it is no longer necessary.
    ExternalFontWinAPI* newFont =
        SF_HEAP_AUTO_NEW(this) ExternalFontWinAPI(this, &SysData, name, fontFlags, &FontLock);
    if (newFont && !newFont->IsValid())
    {
        newFont->Release();
        return 0;
    }

    Font::NativeHintingType* nh = findNativeHinting(name);
    if (nh)
        newFont->SetHinting(*nh);
    else
        newFont->SetHinting(NativeHinting[0]);

    return newFont;
}

//------------------------------------------------------------------------
static int CALLBACK LoadFontNamesProc(
                               ENUMLOGFONTEXW *lpelfe, // logical-font data
                               NEWTEXTMETRICEXW*, // physical-font data
                               DWORD, // type of font
                               LPARAM lParam // application-defined data
                               )
{
    StringHash<String>* pfontnames = static_cast<StringHash<String>*>((void*)lParam);
    String fontname(lpelfe->elfFullName);
    pfontnames->Set(fontname, fontname);
    return 1;
}

void FontProviderWinAPI::LoadFontNames(StringHash<String>& fontnames)
{
    LOGFONTW lf;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfFaceName[0] = '\0';
    lf.lfPitchAndFamily = 0;
    EnumFontFamiliesExW(SysData.WinHDC, &lf, (FONTENUMPROCW)LoadFontNamesProc, (LPARAM)&fontnames, 0);
}

}} // Scaleform::Render
