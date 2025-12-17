/**************************************************************************

PublicHeader:   FontProvider_PS3
Filename    :   Render_FontProviderPS3.h
Content     :   PS3 Cell font provider
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

#ifndef INC_SF_Render_FontProviderPS3_H
#define INC_SF_Render_FontProviderPS3_H

#include <cell/font/libfont.h>
#include <cell/font/libfontFT.h>
#include <cell/font/ps3fontset.h>

#include "Kernel/SF_Threads.h"
#include "Render/Render_Font.h"

namespace Scaleform { namespace Render
{

//------------------------------------------------------------------------
class FontProviderPS3;

struct FontInfoPS3
{
    String          FontName;
    unsigned        FontFlags;
    String          FileName;
    const char*     FontData;
    unsigned        FontDataSize;
    uint32_t        SubNum;
    int32_t         UniqueId;
    CellFontType    Type;

    Font::NativeHintingRange    RasterHintingRange;
    Font::NativeHintingRange    VectorHintingRange;
    unsigned                    MaxRasterHintedSize;
    unsigned                    MaxVectorHintedSize;
};


class ExternalFontPS3 : public Font
{
    enum { FontHeight = 1024, ShapePageSize = 256-2 - 8-4 };
public:
    ExternalFontPS3(FontProviderPS3* pprovider, const CellFontLibrary* library, FontInfoPS3* fontInfo);

    virtual ~ExternalFontPS3();

    bool    IsValid() const { return FontOK == CELL_OK; }

    virtual int      GetGlyphIndex(UInt16 code);
    virtual bool     IsHintedVectorGlyph(unsigned glyphIndex, unsigned glyphSize) const;
    virtual bool     IsHintedRasterGlyph(unsigned glyphIndex, unsigned glyphSize) const;
    virtual const ShapeDataInterface*  GetPermanentGlyphShape(unsigned glyphIndex) const { SF_UNUSED(glyphIndex); return 0; }
    virtual bool     GetTemporaryGlyphShape(unsigned glyphIndex, unsigned glyphSize, GlyphShape * pshape);
    virtual GlyphRaster* GetGlyphRaster(unsigned glyphIndex, unsigned glyphSize) { SF_UNUSED2(glyphIndex, glyphSize); return 0; }
    virtual float    GetAdvance(unsigned glyphIndex) const;
    virtual float    GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const;

    virtual float    GetGlyphWidth(unsigned glyphIndex) const;
    virtual float    GetGlyphHeight(unsigned glyphIndex) const;
    virtual RectF&   GetGlyphBounds(unsigned glyphIndex, RectF* prect) const;

    virtual const char* GetName() const { return &Name[0]; }

private:
    void    setFontMetrics();
    bool    decomposeGlyphOutline(const CellFontGlyph* glyph, GlyphShape* shape, unsigned fontHeight);

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

    // AddRef for font provider since it contains our cache.
    Ptr<FontProviderPS3>           pFontProvider;    
    String                         Name;
    CellFont                       Font;
    Array<GlyphType>               Glyphs;
    HashIdentity<UInt16, unsigned> CodeTable;
    Hash<KerningPairType, float>   KerningPairs;
    Ptr<GlyphRaster>               pRaster;
    unsigned                       LastFontHeight;
    NativeHintingRange             RasterHintingRange;
    NativeHintingRange             VectorHintingRange;
    unsigned                       MaxRasterHintedSize;
    unsigned                       MaxVectorHintedSize;
    int                            FontOK;
};



//------------------------------------------------------------------------
class FontProviderPS3 : public FontProvider
{
public:
    FontProviderPS3(unsigned fontCacheSize=1024*1024, unsigned numUserFontEntries=8, unsigned firstId=0);
    virtual ~FontProviderPS3();

    void MapSystemFont(const char* fontName, unsigned fontFlags, 
                       uint32_t fontType, uint32_t fontMap,
                       Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                       Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                       unsigned maxVectorHintedSize=24,
                       unsigned maxRasterHintedSize=24);

    void MapFontToFile(const char* fontName, unsigned fontFlags, 
                       const char* fileName, uint32_t subNum=0,
                       Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                       Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                       unsigned maxVectorHintedSize=24,
                       unsigned maxRasterHintedSize=24);

    void MapFontToMemory(const char* fontName, unsigned fontFlags, 
                         const char* fontData, unsigned dataSize, 
                         uint32_t subNum=0,
                         Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                         Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                         unsigned maxVectorHintedSize=24,
                         unsigned maxRasterHintedSize=24);

    virtual Font*       CreateFont(const char* name, unsigned fontFlags);

    virtual void        LoadFontNames(StringHash<String>& fontnames);

private:
    ExternalFontPS3*    createFont(const FontInfoPS3& font);

    Array<uint32_t>         FontFileCache;
    Array<CellFontEntry>    UserFontEntries;
    CellFontConfig          FontConfig;
    CellFontLibraryConfigFT FtConfig;
    const CellFontLibrary*  Library;
    int32_t                 NextFontId;

    int                     FontOK;

    Array<FontInfoPS3>      Fonts;
#ifdef SF_ENABLE_THREADS
    mutable Mutex           LockMutex;
#endif
};

}}

#endif

