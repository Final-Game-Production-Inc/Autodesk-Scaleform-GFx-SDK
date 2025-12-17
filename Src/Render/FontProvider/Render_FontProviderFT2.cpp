/**************************************************************************

Filename    :   Render_FontProviderFT2.cpp
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

#include "Render/FontProvider/Render_FontProviderFT2.h"
#include "Render/FontProvider/Render_FT2Helper.h"
#include "Kernel/SF_String.h"
#include "Render/Render_Math2D.h"

namespace Scaleform { namespace Render {

float GetDefaultGlyphWidth() { return 0; }
float GetDefaultGlyphHeight() { return 0; }

//------------------------------------------------------------------------
bool ExternalFontFT2::decomposeGlyphOutline( const FT_Outline& outline, GlyphShape* shape, unsigned hintedSize )
{   
    FTOutline goutline;
    goutline.NContours  = outline.n_contours;   
    goutline.NPoints    = outline.n_points;       
#ifdef SF_BUILD_DEBUG
    FT_Vector testVector = {1,2};
    FTVector *pgtestVector =  reinterpret_cast<FTVector*>(&testVector);
    SF_ASSERT((pgtestVector->x == 1) && (pgtestVector->y == 2));
#endif
    goutline.Points     = reinterpret_cast<FTVector*>(outline.points);         
    goutline.Tags       = outline.tags;   
    goutline.Contours   = outline.contours; 
    goutline.Flags      = outline.flags;   

    return FT2Helper::decomposeGlyphOutline(goutline, shape, hintedSize); 
}


//------------------------------------------------------------------------
class BitsetIterator
{
public:
    BitsetIterator(const UByte* bits, unsigned offset = 0) :
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
void ExternalFontFT2::decomposeGlyphBitmap(const FT_Bitmap& bitmap, int x, int y,
                                           GlyphRaster* raster)
{
    raster->Width   = bitmap.width;
    raster->Height  = bitmap.rows;
    raster->OriginX = -x;
    raster->OriginY =  y;
    raster->Raster.Resize(raster->Width * raster->Height);

    int i;
    int pitch = bitmap.pitch;

    const UByte* src = (const UByte*)bitmap.buffer;
          UByte* dst = &raster->Raster[0];
    for(i = 0; i < bitmap.rows; i++)
    {
        BitsetIterator bits(src, 0);
        int j;
        for(j = 0; j < bitmap.width; j++)
        {
            *dst++ = bits.GetBit() ? 255 : 0;
            ++bits;
        }
        src += pitch;
    }
}

//------------------------------------------------------------------------
ExternalFontFT2::~ExternalFontFT2()
{
    if (Face)
        FT_Done_Face(Face);
}

//------------------------------------------------------------------------
ExternalFontFT2::ExternalFontFT2(FontProviderFT2* pprovider, FT_Library lib, 
                                 const String& fontName, unsigned fontFlags,
                                 const char* fileName, unsigned faceIndex, Lock* plock) :
    Font(fontFlags),
    pFontProvider(pprovider),
    Name(fontName),
    LastFontHeight(FontHeight),
    pFontLock(plock)
{
    int err = FT_New_Face(lib, fileName, faceIndex, &Face);
    if (err)
    {
        Face = 0;
        return;
    }
    setFontMetrics();
}

//------------------------------------------------------------------------
ExternalFontFT2::ExternalFontFT2(FontProviderFT2* pprovider, FT_Library lib, 
                                 const String& fontName, unsigned fontFlags,
                                 const char* fontMem, unsigned fontMemSize, 
                                 unsigned faceIndex,
                                 Lock* plock) :
    Font(fontFlags),
    pFontProvider(pprovider),
    Name(fontName),
    pFontLock(plock)
{
    int err = FT_New_Memory_Face(lib, 
                                 (const FT_Byte*)fontMem, 
                                 fontMemSize, 
                                 faceIndex,
                                 &Face);
    if (err)
    {
        Face = 0;
        return;
    }
    setFontMetrics();
}

//------------------------------------------------------------------------
void ExternalFontFT2::SetHinting(NativeHintingRange vectorHintingRange,
                                 NativeHintingRange rasterHintingRange, 
                                 unsigned maxVectorHintedSize,
                                 unsigned maxRasterHintedSize)
{
    VectorHintingRange  = vectorHintingRange;
    RasterHintingRange  = rasterHintingRange;
    MaxVectorHintedSize = maxVectorHintedSize;
    MaxRasterHintedSize = maxRasterHintedSize;
}

//------------------------------------------------------------------------
void ExternalFontFT2::setFontMetrics()
{
    FT_Set_Pixel_Sizes(Face, FontHeight, FontHeight);
    LastFontHeight = FontHeight;
    float ascent  =  float(Face->ascender)  * FontHeight / Face->units_per_EM;
    float descent = -float(Face->descender) * FontHeight / Face->units_per_EM;
    float height  =  float(Face->height)    * FontHeight / Face->units_per_EM;
    SetFontMetrics(height - ascent + descent, ascent, descent);
}


//------------------------------------------------------------------------
int ExternalFontFT2::GetGlyphIndex(UInt16 code)
{
    if (Face)
    {
        Lock::Locker locker(pFontLock);
        const unsigned* indexPtr = CodeTable.Get(code);
        if (indexPtr)
            return *indexPtr;

        ExternalFontFT2* pthis = const_cast<ExternalFontFT2*>(this);
        if (LastFontHeight != FontHeight)
        {
            // FT_Set_Pixel_Sizes is expensive. Avoid calling it often.
            FT_Set_Pixel_Sizes(Face, FontHeight, FontHeight);
            pthis->LastFontHeight = FontHeight;
        }
        unsigned ftIndex = FT_Get_Char_Index(Face, code);
        int  err     = FT_Load_Glyph(Face, ftIndex, FT_LOAD_NO_HINTING);

        if (err)
            return -1;

        GlyphType glyph;
        glyph.Code          =  code;
        glyph.FtIndex       =  ftIndex;
        glyph.Advance       =  float((Face->glyph->advance.x + 32) >> 6);

        glyph.Bounds.x1 =  float(Face->glyph->metrics.horiBearingX >> 6);
        glyph.Bounds.y1 = -float(Face->glyph->metrics.horiBearingY >> 6);
        glyph.Bounds.x2 =  float(Face->glyph->metrics.width  >> 6) + glyph.Bounds.x1;
        glyph.Bounds.y2 =  float(Face->glyph->metrics.height >> 6) + glyph.Bounds.y1;

        pthis->Glyphs.PushBack(glyph);
        pthis->CodeTable.Add(code, (unsigned)Glyphs.GetSize()-1);
        return (unsigned)Glyphs.GetSize()-1;
    }
    return -1;
}

//------------------------------------------------------------------------
bool ExternalFontFT2::IsHintedVectorGlyph(unsigned glyphIndex, unsigned glyphSize) const
{
    if (IsMissingGlyph(glyphIndex) ||
        glyphSize > MaxVectorHintedSize ||
        VectorHintingRange == DontHint)
    {
        return false;
    }

    if (VectorHintingRange == HintAll)
        return true;

    Lock::Locker locker(pFontLock);
    return IsCJK(UInt16(Glyphs[glyphIndex].Code));
}

//------------------------------------------------------------------------
bool ExternalFontFT2::IsHintedRasterGlyph(unsigned glyphIndex, unsigned glyphSize) const
{
    if (IsMissingGlyph(glyphIndex) ||
        glyphSize > MaxRasterHintedSize ||
        RasterHintingRange == DontHint)
    {
        return false;
    }

    if (RasterHintingRange == HintAll)
        return true;
    Lock::Locker locker(pFontLock);
    return IsCJK(UInt16(Glyphs[glyphIndex].Code));
}

//------------------------------------------------------------------------
bool ExternalFontFT2::GetTemporaryGlyphShape(unsigned glyphIndex,
                                             unsigned glyphSize, 
                                             GlyphShape * pshape)
{
    if (IsMissingGlyph(glyphIndex))
        return 0;

    if (!IsHintedVectorGlyph(glyphIndex, glyphSize))
        glyphSize = 0;
    Lock::Locker locker(pFontLock);
    unsigned pixelSize = glyphSize ? glyphSize : FontHeight;

    if (LastFontHeight != pixelSize)
    {
        // FT_Set_Pixel_Sizes is expensive. Avoid calling it often.
        FT_Set_Pixel_Sizes(Face, pixelSize, pixelSize);
        LastFontHeight = pixelSize;
    }

    const GlyphType& glyph = Glyphs[glyphIndex];
    int err = FT_Load_Glyph(Face, glyph.FtIndex, FT_LOAD_DEFAULT);
    if (err)
        return 0;

    if (!decomposeGlyphOutline(Face->glyph->outline, pshape, glyphSize ))
        return false;

    return true;
}

//------------------------------------------------------------------------
bool ExternalFontFT2::GetGlyphRaster(unsigned glyphIndex, unsigned glyphSize, GlyphRaster* raster)
{
    if (!IsHintedRasterGlyph(glyphIndex, glyphSize))
        return false;
    Lock::Locker locker(pFontLock);
    GlyphType& glyph = Glyphs[glyphIndex];

    if (LastFontHeight != glyphSize)
    {
        // FT_Set_Pixel_Sizes is expensive. Avoid calling it often.
        FT_Set_Pixel_Sizes(Face, glyphSize, glyphSize);
        LastFontHeight = glyphSize;
    }

    int err = FT_Load_Glyph(Face, glyph.FtIndex, FT_LOAD_DEFAULT);
    if (err)
        return false;

    err = FT_Render_Glyph(Face->glyph, FT_RENDER_MODE_MONO);
    if (err)
        return false;

    decomposeGlyphBitmap(Face->glyph->bitmap, 
                         Face->glyph->bitmap_left,
                         Face->glyph->bitmap_top,
                         raster);
    return true;
}

//------------------------------------------------------------------------
float ExternalFontFT2::GetAdvance(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex))
        return GetDefaultGlyphWidth();

    Lock::Locker locker(pFontLock);
    return Glyphs[glyphIndex].Advance;
}

//------------------------------------------------------------------------
float ExternalFontFT2::GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const
{
    Lock::Locker locker(pFontLock);
    if(Face && FT_HAS_KERNING(Face))
    {
        FT_Vector delta;
        FT_Get_Kerning(Face, 
                       FT_Get_Char_Index(Face, lastCode), 
                       FT_Get_Char_Index(Face, thisCode),
                       FT_KERNING_DEFAULT, &delta);
        return float(delta.x >> 6);
    }
    return 0;
}

//------------------------------------------------------------------------
float ExternalFontFT2::GetGlyphWidth(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex))
        return GetDefaultGlyphWidth();

    Lock::Locker locker(pFontLock);
    const RectF& r = Glyphs[glyphIndex].Bounds;
    return r.Width();
}

//------------------------------------------------------------------------
float ExternalFontFT2::GetGlyphHeight(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex))
        return GetDefaultGlyphHeight();

    Lock::Locker locker(pFontLock);
    const RectF& r = Glyphs[glyphIndex].Bounds;
    return r.Height();
}

//------------------------------------------------------------------------
RectF& ExternalFontFT2::GetGlyphBounds(unsigned glyphIndex, RectF* prect) const
{
    if (IsMissingGlyph(glyphIndex))
        prect->SetRect(GetDefaultGlyphWidth(), GetDefaultGlyphHeight());
    else
    {
        Lock::Locker locker(pFontLock);
        *prect = Glyphs[glyphIndex].Bounds;
    }
    return *prect;
}





//------------------------------------------------------------------------
FontProviderFT2::FontProviderFT2(FT_Library lib):
    Lib(lib),
    ExtLibFlag(true),
    NamesEndIdx(0)
{
    if (Lib == 0)
    {
        int err = FT_Init_FreeType(&Lib);
        if (err) 
            Lib = 0;
        ExtLibFlag = false;
    }
}

//------------------------------------------------------------------------
FontProviderFT2::~FontProviderFT2()
{
    if (Lib && !ExtLibFlag)
        FT_Done_FreeType(Lib);
}

//------------------------------------------------------------------------
void FontProviderFT2::MapFontToFile(const char* fontName, unsigned fontFlags,
                                    const char* fileName, unsigned faceIndex,
                                    Font::NativeHintingRange vectorHintingRange,
                                    Font::NativeHintingRange rasterHintingRange, 
                                    unsigned maxVectorHintedSize,
                                    unsigned maxRasterHintedSize)
{
    // Mask flags to be safe.
    fontFlags &= Font::FF_CreateFont_Mask;
    fontFlags |= Font::FF_DeviceFont | Font::FF_NativeHinting;

    FontType font;
    font.FontName     = fontName;
    font.FontFlags    = fontFlags;
    font.FileName     = fileName;
    font.FontData     = 0;
    font.FontDataSize = 0;
    font.FaceIndex    = faceIndex;
    font.VectorHintingRange  = vectorHintingRange;
    font.RasterHintingRange  = rasterHintingRange;
    font.MaxVectorHintedSize = maxVectorHintedSize;
    font.MaxRasterHintedSize = maxRasterHintedSize;
    Fonts.PushBack(font);
}

//------------------------------------------------------------------------
void FontProviderFT2::MapFontToMemory(const char* fontName, unsigned fontFlags, 
                                      const char* fontData, unsigned dataSize, unsigned faceIndex,
                                      Font::NativeHintingRange vectorHintingRange,
                                      Font::NativeHintingRange rasterHintingRange, 
                                      unsigned maxVectorHintedSize,
                                      unsigned maxRasterHintedSize)
{
    // Mask flags to be safe.
    fontFlags &= Font::FF_CreateFont_Mask;
    fontFlags |= Font::FF_DeviceFont | Font::FF_NativeHinting;

    FontType font;
    font.FontName     = fontName;
    font.FontFlags    = fontFlags;
    font.FontData     = fontData;
    font.FontDataSize = dataSize;
    font.FaceIndex    = faceIndex;
    font.VectorHintingRange  = vectorHintingRange;
    font.RasterHintingRange  = rasterHintingRange;
    font.MaxVectorHintedSize = maxVectorHintedSize;
    font.MaxRasterHintedSize = maxRasterHintedSize;
    Fonts.PushBack(font);
}

//------------------------------------------------------------------------
ExternalFontFT2* FontProviderFT2::createFont(const FontType& font)
{
    ExternalFontFT2* newFont = font.FontData?
        new ExternalFontFT2(this, Lib, font.FontName, font.FontFlags, font.FontData, font.FontDataSize, font.FaceIndex, &FontLock):
        new ExternalFontFT2(this, Lib, font.FontName, font.FontFlags, font.FileName, font.FaceIndex, &FontLock);

    if (newFont && !newFont->IsValid())
    {
        newFont->Release();
        return 0;
    }
    newFont->SetHinting(font.VectorHintingRange,  font.RasterHintingRange,
                        font.MaxVectorHintedSize, font.MaxRasterHintedSize);
    return newFont;
}

//------------------------------------------------------------------------
Font* FontProviderFT2::CreateFont(const char* name, unsigned fontFlags)
{
    if (Lib == 0)
        return 0;

    Lock::Locker lock(&FontLock);

    // Mask flags to be safe.
    fontFlags &= Font::FF_CreateFont_Mask;
    fontFlags |= Font::FF_DeviceFont | Font::FF_NativeHinting;

    unsigned i;
    for (i = 0; i < Fonts.GetSize(); ++i)
    {
        FontType& font = Fonts[i];
        if (font.FontName.CompareNoCase(name) == 0 && 
           (font.FontFlags & Font::FF_Style_Mask) == (fontFlags & Font::FF_Style_Mask))
        {
            return createFont(font);
        }
    }
    return 0;
}

//------------------------------------------------------------------------
void FontProviderFT2::LoadFontNames(StringHash<String>& fontnames)
{
    for (unsigned i = 0; i < Fonts.GetSize(); ++i)
    {
        fontnames.Set(Fonts[i].FontName, Fonts[i].FontName);
    }
}

}}
