/**************************************************************************

PublicHeader:   FontProvider_FT2
Filename    :   Render_FontProviderFT2.h
Content     :   FreeType2 font provider
Created     :   6/21/2007
Authors     :   Maxim Shemanarev

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
**************************************************************************/

#ifndef INC_SF_Render_FontProviderFT2_H
#define INC_SF_Render_FontProviderFT2_H

#include "ft2build.h"
#include FT_FREETYPE_H

#include "Kernel/SF_Threads.h"
#include "Render/Render_Font.h"

namespace Scaleform { namespace Render { 

//------------------------------------------------------------------------
class FontProviderFT2;

class ExternalFontFT2 : public Font
{
    enum { FontHeight = 1024, ShapePageSize = 256-2 - 8-4 };
public:
    ExternalFontFT2(FontProviderFT2* pprovider, 
                    FT_Library lib, 
                    const String& fontName, 
                    unsigned fontFlags,
                    const char* fileName, 
                    unsigned faceIndex,
                    Lock* plock);

    ExternalFontFT2(FontProviderFT2* pprovider, 
                    FT_Library lib, 
                    const String& fontName, 
                    unsigned fontFlags,
                    const char* fontMem, 
                    unsigned fontMemSize, 
                    unsigned faceIndex,
                    Lock* plock);

    void SetHinting(NativeHintingRange vectorHintingRange,
                    NativeHintingRange rasterHintingRange, 
                    unsigned maxVectorHintedSize,
                    unsigned maxRasterHintedSize);

    virtual ~ExternalFontFT2();

    bool    IsValid() const { return Face != 0; }
    
    virtual int      GetGlyphIndex(UInt16 code);
    virtual bool     IsHintedVectorGlyph(unsigned glyphIndex, unsigned glyphSize) const;
    virtual bool     IsHintedRasterGlyph(unsigned glyphIndex, unsigned glyphSize) const;
    virtual const ShapeDataInterface*  GetPermanentGlyphShape(unsigned glyphIndex) const { SF_UNUSED(glyphIndex); return 0; }
    virtual bool     GetTemporaryGlyphShape(unsigned glyphIndex, unsigned glyphSize, GlyphShape * pshape);
    virtual bool     GetGlyphRaster(unsigned glyphIndex, unsigned hintedSize, GlyphRaster* raster);
    virtual float    GetAdvance(unsigned glyphIndex) const;
    virtual float    GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const;

    virtual float    GetGlyphWidth(unsigned glyphIndex) const;
    virtual float    GetGlyphHeight(unsigned glyphIndex) const;
    virtual RectF&   GetGlyphBounds(unsigned glyphIndex, RectF* prect) const;

    virtual const char* GetName() const { return &Name[0]; }

private:
    void    setFontMetrics();
    bool    decomposeGlyphOutline(const FT_Outline& outline, GlyphShape* shape, unsigned hintedSize);
    void    decomposeGlyphBitmap(const FT_Bitmap& bitmap, int x, int y, GlyphRaster* raster);

    static inline int SF_STDCALL FtToTwips(int v)
    {
        return (v * 20) >> 6;
    }

    static inline int SF_STDCALL FtToS1024(int v)
    {
        return v >> 6;
    }

    struct GlyphType
    {
        unsigned                Code;
        unsigned                FtIndex;
        float                   Advance;
        RectF                  Bounds;
    };

    struct KerningPairType
    {           
        UInt16  Char0, Char1;
        bool    operator==(const KerningPairType& k) const
            { return Char0 == k.Char0 && Char1 == k.Char1; }
    };

    // AddRef for font provider since it contains our cache.
    Ptr<FontProviderFT2>            pFontProvider;    
    String                          Name;
    FT_Face                         Face;
    Array<GlyphType>                Glyphs;
    HashIdentity<UInt16, unsigned>  CodeTable;
    Hash<KerningPairType, float>    KerningPairs;
    unsigned                        LastFontHeight;
    NativeHintingRange              RasterHintingRange;
    NativeHintingRange              VectorHintingRange;
    unsigned                        MaxRasterHintedSize;
    unsigned                        MaxVectorHintedSize;

    mutable Lock*                   pFontLock;

};



//------------------------------------------------------------------------
class FontProviderFT2 : public FontProvider
{
public:
    FontProviderFT2(FT_Library lib=0);
    virtual ~FontProviderFT2();

    FT_Library GetFT_Library() { return Lib; }

    void MapFontToFile(const char* fontName, unsigned fontFlags, 
                       const char* fileName, unsigned faceIndex=0, 
                       Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                       Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                       unsigned maxVectorHintedSize=24,
                       unsigned maxRasterHintedSize=24);

    void MapFontToMemory(const char* fontName, unsigned fontFlags, 
                         const char* fontData, unsigned dataSize, 
                         unsigned faceIndex=0, 
                         Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                         Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                         unsigned maxVectorHintedSize=24,
                         unsigned maxRasterHintedSize=24);

    virtual Font*   CreateFont(const char* name, unsigned fontFlags);

    virtual void    LoadFontNames(StringHash<String>& fontnames);

private:
    struct FontType
    {
        String                      FontName;
        unsigned                    FontFlags;
        String                      FileName;
        const char*                 FontData;
        unsigned                    FontDataSize;
        unsigned                    FaceIndex;
        Font::NativeHintingRange    RasterHintingRange;
        Font::NativeHintingRange    VectorHintingRange;
        unsigned                    MaxRasterHintedSize;
        unsigned                    MaxVectorHintedSize;
    };

    ExternalFontFT2* createFont(const FontType& font);

    FT_Library          Lib;
    bool                ExtLibFlag;
    Array<FontType>     Fonts;
    unsigned            NamesEndIdx;
#ifdef SF_ENABLE_THREADS
    mutable Lock       FontLock;
#endif
};

}}

#endif
