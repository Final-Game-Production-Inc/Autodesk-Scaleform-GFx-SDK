/**************************************************************************

PublicHeader:   Render
Filename    :   Render_FontProviderWinAPI.h
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

#ifndef INC_SF_Render_FontProviderWinAPI_H
#define INC_SF_Render_FontProviderWinAPI_H

#include <windows.h>
#include "Kernel/SF_Hash.h"
#include "Render/Render_Font.h"

namespace Scaleform { namespace Render {

//------------------------------------------------------------------------
struct FontSysDataWinAPI
{
    enum { BufSizeInc = 1024-8 };

    Array<UByte>    GlyphBuffer;
    HDC             WinHDC;

    FontSysDataWinAPI(HDC dc) : WinHDC(dc) {}
};



//------------------------------------------------------------------------
class FontProviderWinAPI;

class ExternalFontWinAPI : public Font
{
    // GGO_FontHeight is the font height to request the 
    // glyphs with the nominal size (not the specific hinted size). 
    // It appears to be extremely expensive to call GetGlyphOutline
    // with font height of 256 or more. So, the GGO_FontHeight
    // is the "maximum inexpensive size" for GetGlyphOutline.
    enum 
    { 
        GGO_FontHeight = 240
    };
public:
    ExternalFontWinAPI(FontProviderWinAPI *pprovider,
                       FontSysDataWinAPI* pSysData, 
                       const char* name, unsigned fontFlags, 
                       Lock* fontLock);

    virtual ~ExternalFontWinAPI();

    bool    IsValid() const { return MasterFont != 0; }
    void    SetHinting(const Font::NativeHintingType& pnh);

    virtual const char* GetName() const;

    virtual int     GetGlyphIndex(UInt16 code);
    virtual float   GetAdvance(unsigned glyphIndex) const;
    virtual float   GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const;
    virtual float   GetGlyphWidth(unsigned glyphIndex) const;
    virtual float   GetGlyphHeight(unsigned glyphIndex) const;
    virtual RectF&  GetGlyphBounds(unsigned glyphIndex, RectF* prect) const;
    virtual bool    IsHintedVectorGlyph(unsigned glyphIndex, unsigned hintedSize) const;
    virtual bool    IsHintedRasterGlyph(unsigned glyphIndex, unsigned hintedSize) const;
    virtual const ShapeDataInterface* GetPermanentGlyphShape(unsigned glyphIndex) const { SF_UNUSED(glyphIndex); return 0; }
    virtual bool    GetTemporaryGlyphShape(unsigned glyphIndex, unsigned hintedSize, GlyphShape* shape);
    virtual bool    GetGlyphRaster(unsigned glyphIndex, unsigned hintedSize, GlyphRaster* raster);

private:
    bool    decomposeGlyphOutline(const UByte* data, unsigned size, GlyphShape* shape, unsigned hintedSize);
    void    decomposeGlyphBitmap(const UByte* data, int w, int h, int x, int y, GlyphRaster* raster);
    void    loadKerningPairs();

    static inline float FxToTwips(FIXED v)
    {
        return float(v.value) * 20.0f + float(v.fract) * 20.0f / 65536.0f;
    }

    static inline float FxToS1024(FIXED v)
    {
        return float((int(v.value) << 8) + (int(v.fract) >> 8)) * 4.0f / float(GGO_FontHeight);
    }

    struct GlyphType
    {
        unsigned    Code;
        float       Advance;
        RectF       Bounds;
    };

    struct KerningPairType
    {           
        UInt16  Char0, Char1;
        bool    operator==(const KerningPairType& k) const
            { return Char0 == k.Char0 && Char1 == k.Char1; }
    };

    // AddRef for font provider since it contains our SysData.
    Ptr<FontProviderWinAPI>         pFontProvider;    
    FontSysDataWinAPI*              pSysData;
    ArrayLH<char>                   Name; 
    ArrayLH<WCHAR>                  NameW;
    HFONT                           MasterFont;
    HFONT                           HintedFont;
    unsigned                        LastHintedFontSize;
    ArrayLH<GlyphType>              Glyphs;
    HashIdentityLH<UInt16, unsigned> CodeTable;
    HashLH<KerningPairType, float>  KerningPairs;
    float                           Scale1024;
    NativeHintingType               Hinting;
    Lock*                           pFontLock;
};



//------------------------------------------------------------------------
class FontProviderWinAPI : public FontProvider
{
public:
    FontProviderWinAPI(HDC dc);
    virtual ~FontProviderWinAPI();

    void SetHintingAllFonts(Font::NativeHintingRange vectorRange, 
                            Font::NativeHintingRange rasterRange, 
                            unsigned maxVectorHintedSize=24,
                            unsigned maxRasterHintedSize=24);
    void SetHinting(const char* name, 
                    Font::NativeHintingRange vectorRange, 
                    Font::NativeHintingRange rasterRange, 
                    unsigned maxVectorHintedSize=24,
                    unsigned maxRasterHintedSize=24);
    
    virtual Font*    CreateFont(const char* name, unsigned fontFlags);    

    virtual void     LoadFontNames(StringHash<String>& fontnames);

private:
    Font::NativeHintingType* findNativeHinting(const char* name);

    FontSysDataWinAPI               SysData;
    Array<Font::NativeHintingType>  NativeHinting;
    Lock                            FontLock;
};

}} // Scaleform::Render

#endif
