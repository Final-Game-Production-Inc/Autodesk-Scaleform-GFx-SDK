/**************************************************************************

Filename    :   Render_FontProviderPS3.cpp
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

#include <cell/sysmodule.h>
#include "Render/FontProvider/Render_FontProviderPS3.h"
#include "Kernel/SF_String.h"
#include "Render/Render_Math2D.h"

#define PS3_CURVE_TAG(flag) (flag & 3)
#define PS3_CURVE_TAG_ON     1
#define PS3_CURVE_TAG_CONIC  0
#define PS3_CURVE_TAG_CUBIC  2

namespace Scaleform { namespace Render
{

//------------------------------------------------------------------------
bool ExternalFontPS3::decomposeGlyphOutline(const CellFontGlyph* glyph, GlyphShape* shape, unsigned fontHeight)
{   
    struct PointType { float x, y; };
    const CellFontGlyphOutline& outline = glyph->Outline;
    PointType v_last;
    PointType v_control;
    PointType v_start;

    SF_UNUSED(fontHeight);
    float ratio = 1.0f; // TO DO: Revise, consider fontHeight

    const PointType*  point;
    const PointType*  limit;
    const uint8_t*    tags;

    int  n;         // index of contour in outline
    int  first = 0; // index of first point in contour
    char tag;       // current point's state

    ShapePosInfo pos(0);
    shape->Clear();

    for(n = 0; n < outline.contoursCount; n++)
    {
        int  last;  // index of last point in contour

        last  = outline.contourIndexs[n];
        limit = (const PointType*)outline.Points + last;

        v_start.x = outline.Points[first].x;
        v_start.y = outline.Points[first].y;
        v_last.x  = outline.Points[last].x;
        v_last.y  = outline.Points[last].y;

        v_control = v_start;

        point = (const PointType*)outline.Points + first;
        tags  = outline.pointTags  + first;
        tag   = PS3_CURVE_TAG(tags[0]);

        // A contour cannot start with a cubic control point!
        if(tag == PS3_CURVE_TAG_CUBIC) return false;

        // check first point to determine origin
        if( tag == PS3_CURVE_TAG_CONIC)
        {
            // first point is conic control.  Yes, this happens.
            if(PS3_CURVE_TAG(outline.pointTags[last]) == PS3_CURVE_TAG_ON)
            {
                // start at last point if it is on the curve
                v_start = v_last;
                limit--;
            }
            else
            {
                // if both first and last points are conic,
                // start at their middle and record its position
                // for closure
                v_start.x = (v_start.x + v_last.x) / 2;
                v_start.y = (v_start.y + v_last.y) / 2;
                v_last = v_start;
            }
            point--;
            tags--;
        }

        shape->StartPath(&pos, Shape_NewPath, 1, 0, 0, ratio*v_start.x, -ratio*v_start.y);

        while(point < limit)
        {
            point++;
            tags++;

            tag = PS3_CURVE_TAG(tags[0]);
            switch(tag)
            {
                case PS3_CURVE_TAG_ON:  // emit a single line_to
                {
                    shape->LineTo(&pos, ratio*point->x, -ratio*point->y);
                    continue;
                }

                case PS3_CURVE_TAG_CONIC:  // consume conic arcs
                {
                    v_control.x = point->x;
                    v_control.y = point->y;

                Do_Conic:
                    if(point < limit)
                    {
                        PointType vec;
                        PointType v_middle;

                        point++;
                        tags++;
                        tag = PS3_CURVE_TAG(tags[0]);

                        vec.x = point->x;
                        vec.y = point->y;

                        if(tag == PS3_CURVE_TAG_ON)
                        {
                            shape->QuadTo(&pos, ratio * v_control.x, -ratio * v_control.y,
                                                ratio * vec.x,       -ratio * vec.y);
                            continue;
                        }

                        if(tag != PS3_CURVE_TAG_CONIC) return false;

                        v_middle.x = (v_control.x + vec.x) / 2;
                        v_middle.y = (v_control.y + vec.y) / 2;
                        shape->QuadTo(&pos, ratio * v_control.x, -ratio * v_control.y,
                                            ratio * v_middle.x,  -ratio * v_middle.y);
                        v_control = vec;
                        goto Do_Conic;
                    }
                    shape->QuadTo(&pos, ratio * v_control.x, -ratio * v_control.y,
                                        ratio * v_start.x,   -ratio * v_start.y);
                    goto Close;
                }

                default:  // PS3_CURVE_TAG_CUBIC
                {
                    PointType vec1, vec2;
                    if(point + 1 > limit || PS3_CURVE_TAG(tags[1]) != PS3_CURVE_TAG_CUBIC)
                    {
                        return false;
                    }

                    vec1 = point[0];
                    vec2 = point[1];

                    point += 2;
                    tags  += 2;

                    if(point <= limit)
                    {
                        PointType vec;
                        vec = *point;
                        shape->QuadTo(&pos, ratio * (vec1.x + vec2.x) / 2, -ratio * (vec1.y + vec2.y) / 2,
                                            ratio * vec.x, -ratio * vec.y);
                        continue;
                    }
                    shape->QuadTo(&pos, ratio * (vec1.x + vec2.x) / 2, -ratio * (vec1.y + vec2.y) / 2,
                                        ratio * v_start.x, -ratio * v_start.y);
                    goto Close;
                }
            }
        }

    Close:
        shape->ClosePath(&pos);
        shape->EndPath();
        first = last + 1; 
    }

    if (!shape->IsEmpty())
    {
        shape->EndShape();
        return true;
    }

    return false;
}


//------------------------------------------------------------------------
ExternalFontPS3::~ExternalFontPS3()
{
    if (FontOK == CELL_OK)
        cellFontCloseFont(&Font);
}

//------------------------------------------------------------------------
ExternalFontPS3::ExternalFontPS3(FontProviderPS3* pprovider, const CellFontLibrary* library, 
                                 FontInfoPS3* fontInfo) :
    Render::Font(fontInfo->FontFlags),
    pFontProvider(pprovider),
    Name(fontInfo->FontName),
    LastFontHeight(FontHeight),
    RasterHintingRange(fontInfo->RasterHintingRange),
    VectorHintingRange(fontInfo->VectorHintingRange),
    MaxRasterHintedSize(fontInfo->MaxRasterHintedSize),
    MaxVectorHintedSize(fontInfo->MaxVectorHintedSize),
    FontOK(-1)
{
    if (fontInfo->FontData)
    {
        FontOK = cellFontOpenFontMemory(
            library, 
            (void*)fontInfo->FontData,
            fontInfo->FontDataSize,
            fontInfo->SubNum,
            fontInfo->UniqueId,
            &Font);
    }
    else
    if (!fontInfo->FileName.IsEmpty())
    {
        FontOK = cellFontOpenFontFile(
            library, 
            (uint8_t*)(fontInfo->FileName.ToCStr()),
            fontInfo->SubNum,
            fontInfo->UniqueId,
            &Font);
    }
    else
    {
        FontOK = cellFontOpenFontset(library, &fontInfo->Type, &Font);
    }

    if(FontOK == CELL_OK)
        setFontMetrics();
}

//------------------------------------------------------------------------
void ExternalFontPS3::setFontMetrics()
{
    cellFontSetScalePixel(&Font, FontHeight, FontHeight);
    LastFontHeight = FontHeight;

    CellFontHorizontalLayout layout;
    cellFontGetHorizontalLayout(&Font, &layout);
    SetFontMetrics(0, layout.baseLineY, layout.lineHeight-layout.baseLineY);
}


//------------------------------------------------------------------------
int ExternalFontPS3::GetGlyphIndex(UInt16 code)
{
    if (FontOK == CELL_OK)
    {
        const unsigned* indexPtr = CodeTable.Get(code);
        if (indexPtr)
            return *indexPtr;

        ExternalFontPS3* pthis = const_cast<ExternalFontPS3*>(this);
        if (LastFontHeight != FontHeight)
        {
            // cellFontSetScalePixel is expensive. Avoid calling it often.
            cellFontSetScalePixel(&pthis->Font, FontHeight, FontHeight);
            pthis->LastFontHeight = FontHeight;
        }
        CellFontGlyphMetrics metrics;
        int err = cellFontGetCharGlyphMetrics(&pthis->Font, code, &metrics);
        if (err != CELL_OK)
            return -1;

        GlyphType glyph;
        glyph.Code      =  code;
        glyph.Advance   =  metrics.Horizontal.advance;
        glyph.Bounds.x1 =  metrics.Horizontal.bearingX;
        glyph.Bounds.y1 = -metrics.Horizontal.bearingY;
        glyph.Bounds.x2 =  glyph.Bounds.x1 + metrics.width;
        glyph.Bounds.y2 =  glyph.Bounds.y1 + metrics.height;

        pthis->Glyphs.PushBack(glyph);
        pthis->CodeTable.Add(code, (unsigned)Glyphs.GetSize()-1);
        return (unsigned)Glyphs.GetSize()-1;
    }
    return -1;
}

//------------------------------------------------------------------------
bool ExternalFontPS3::IsHintedVectorGlyph(unsigned glyphIndex, unsigned glyphSize) const
{
    if (IsMissingGlyph(glyphIndex) ||
        glyphSize > MaxVectorHintedSize ||
        VectorHintingRange == DontHint)
    {
        return false;
    }

    if (VectorHintingRange == HintAll)
        return true;

    return IsCJK(UInt16(Glyphs[glyphIndex].Code));
}

//------------------------------------------------------------------------
bool ExternalFontPS3::IsHintedRasterGlyph(unsigned glyphIndex, unsigned glyphSize) const
{
    if (IsMissingGlyph(glyphIndex) ||
        glyphSize > MaxRasterHintedSize ||
        RasterHintingRange == DontHint)
    {
        return false;
    }

    if (VectorHintingRange == HintAll)
        return true;

    return IsCJK(UInt16(Glyphs[glyphIndex].Code));
}

//------------------------------------------------------------------------
bool ExternalFontPS3::GetTemporaryGlyphShape(unsigned glyphIndex,
                                             unsigned glyphSize, 
                                             GlyphShape * pshape)
{
    if (IsMissingGlyph(glyphIndex))
        return 0;

    if (!IsHintedVectorGlyph(glyphIndex, glyphSize))
        glyphSize = 0;

    unsigned pixelSize = glyphSize ? glyphSize : FontHeight;

    if (LastFontHeight != pixelSize)
    {
        // cellFontSetScalePixel is expensive. Avoid calling it often.
        cellFontSetScalePixel(&Font, (float)pixelSize, (float)pixelSize);
        LastFontHeight = pixelSize;
    }

    CellFontGlyph *glyph;
    int err = cellFontGenerateCharGlyph(&Font, Glyphs[glyphIndex].Code, &glyph);
    if (err)
        return 0;

    if (!decomposeGlyphOutline(glyph, pshape, glyphSize))
        return false;

    return true;
}


//------------------------------------------------------------------------
float ExternalFontPS3::GetAdvance(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex))
        return GetNominalGlyphWidth();

    return Glyphs[glyphIndex].Advance;
}

//------------------------------------------------------------------------
float ExternalFontPS3::GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const
{
    if(FontOK == CELL_OK)
    {
        CellFontKerning kerning;
        CellFont tmpFont = Font;
        int err = cellFontGetKerning(&tmpFont, lastCode, thisCode, &kerning);
        if (err == CELL_OK)
            return kerning.offsetX;
    }
    return 0;
}

//------------------------------------------------------------------------
float ExternalFontPS3::GetGlyphWidth(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex))
        return GetNominalGlyphWidth();

    const RectF& r = Glyphs[glyphIndex].Bounds;
    return r.Width();
}

//------------------------------------------------------------------------
float ExternalFontPS3::GetGlyphHeight(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex))
        return GetNominalGlyphHeight();

    const RectF& r = Glyphs[glyphIndex].Bounds;
    return r.Height();
}

//------------------------------------------------------------------------
RectF& ExternalFontPS3::GetGlyphBounds(unsigned glyphIndex, RectF* prect) const
{
    if (IsMissingGlyph(glyphIndex))
        prect->SetRect(GetNominalGlyphWidth(), GetNominalGlyphHeight());
    else
        *prect = Glyphs[glyphIndex].Bounds;
    return *prect;
}

static void* FontProviderPS3_malloc( void*obj, uint32_t size )
{
    SF_UNUSED(obj);
    return SF_ALLOC(size, Stat_Default_Mem);
}
static void  FontProviderPS3_free( void*obj, void*p )
{
    SF_UNUSED(obj);
    SF_FREE(p);
}
static void* FontProviderPS3_realloc( void*obj, void* p, uint32_t size )
{
    SF_UNUSED(obj);
    return SF_REALLOC(p, size, Stat_Default_Mem);
}
static void* FontProviderPS3_calloc( void*obj, uint32_t numb, uint32_t blockSize )
{
    SF_UNUSED(obj);
    void* p = SF_ALLOC(numb * blockSize, Stat_Default_Mem);
    memset(p, 0, numb * blockSize);
    return p;
}

//------------------------------------------------------------------------
FontProviderPS3::FontProviderPS3(unsigned fontCacheSize, unsigned numUserFontEntries, unsigned firstId)
{
    FontOK = cellSysmoduleLoadModule(CELL_SYSMODULE_FONT);
    if (FontOK == CELL_OK)
        FontOK = cellSysmoduleLoadModule(CELL_SYSMODULE_FREETYPE);
    if (FontOK == CELL_OK)
        cellSysmoduleLoadModule(CELL_SYSMODULE_FONTFT);

    if (FontOK == CELL_OK)
    {
        FontFileCache.Resize(fontCacheSize/4);
        UserFontEntries.Resize(numUserFontEntries);

        FontConfig.FileCache.buffer = &FontFileCache[0]; // Set the file read cache
        FontConfig.FileCache.size   = (uint32_t)(FontFileCache.GetSize() * 4);
        FontConfig.userFontEntrys   = &UserFontEntries[0]; // Set entry for the number of user
        FontConfig.userFontEntryMax = (uint32_t)UserFontEntries.GetSize();
        FontOK = cellFontInit(&FontConfig);
        if(FontOK == CELL_OK)
        {
            CellFontLibraryConfigFT_initialize(&FtConfig); // Initial setting

            FtConfig.MemoryIF.Object  = NULL;
            FtConfig.MemoryIF.Malloc  = FontProviderPS3_malloc;
            FtConfig.MemoryIF.Free    = FontProviderPS3_free;
            FtConfig.MemoryIF.Realloc = FontProviderPS3_realloc;
            FtConfig.MemoryIF.Calloc  = FontProviderPS3_calloc;

            FontOK = cellFontInitLibraryFreeType(&FtConfig, &Library);
            if (FontOK != CELL_OK) 
            {
                cellFontEnd();
            }
        }
    }

    NextFontId = firstId;
}

//------------------------------------------------------------------------
FontProviderPS3::~FontProviderPS3()
{
    if (FontOK == CELL_OK) 
    {
        FontOK = cellFontEndLibrary(Library);
        if (FontOK == CELL_OK)
        {
            cellFontEnd();
        }
    }
}

//------------------------------------------------------------------------
void FontProviderPS3::MapSystemFont(const char* fontName, unsigned fontFlags,
                                    uint32_t fontType, uint32_t fontMap,
                                    Font::NativeHintingRange vectorHintingRange,
                                    Font::NativeHintingRange rasterHintingRange, 
                                    unsigned maxVectorHintedSize,
                                    unsigned maxRasterHintedSize)
{
    // Mask flags to be safe.
    fontFlags &= Font::FF_CreateFont_Mask;
    fontFlags |= Font::FF_DeviceFont | Font::FF_NativeHinting;

    FontInfoPS3 font;
    font.FontName     = fontName;
    font.FontFlags    = fontFlags;
    font.FontData     = 0;
    font.FontDataSize = 0;
    font.SubNum       = 0;
    font.UniqueId     = 0;
    font.Type.map     = fontMap;
    font.Type.type    = fontType;
    font.VectorHintingRange  = vectorHintingRange;
    font.RasterHintingRange  = rasterHintingRange;
    font.MaxVectorHintedSize = maxVectorHintedSize;
    font.MaxRasterHintedSize = maxRasterHintedSize;
    Fonts.PushBack(font);
}

//------------------------------------------------------------------------
void FontProviderPS3::MapFontToFile(const char* fontName, unsigned fontFlags,
                                    const char* fileName, uint32_t subNum,
                                    Font::NativeHintingRange vectorHintingRange,
                                    Font::NativeHintingRange rasterHintingRange, 
                                    unsigned maxVectorHintedSize,
                                    unsigned maxRasterHintedSize)
{
    // Mask flags to be safe.
    fontFlags &= Font::FF_CreateFont_Mask;
    fontFlags |= Font::FF_DeviceFont | Font::FF_NativeHinting;

    FontInfoPS3 font;
    font.FontName     = fontName;
    font.FontFlags    = fontFlags;
    font.FileName     = fileName;
    font.FontData     = 0;
    font.FontDataSize = 0;
    font.SubNum       = subNum;
    font.UniqueId     = NextFontId++;
    font.Type.map     = ~0U;
    font.Type.type    = ~0U;
    font.VectorHintingRange  = vectorHintingRange;
    font.RasterHintingRange  = rasterHintingRange;
    font.MaxVectorHintedSize = maxVectorHintedSize;
    font.MaxRasterHintedSize = maxRasterHintedSize;
    Fonts.PushBack(font);
}

//------------------------------------------------------------------------
void FontProviderPS3::MapFontToMemory(const char* fontName, unsigned fontFlags, 
                                      const char* fontData, unsigned dataSize,
                                      uint32_t subNum,
                                      Font::NativeHintingRange vectorHintingRange,
                                      Font::NativeHintingRange rasterHintingRange, 
                                      unsigned maxVectorHintedSize,
                                      unsigned maxRasterHintedSize)
{
    // Mask flags to be safe.
    fontFlags &= Font::FF_CreateFont_Mask;
    fontFlags |= Font::FF_DeviceFont | Font::FF_NativeHinting;

    FontInfoPS3 font;
    font.FontName     = fontName;
    font.FontFlags    = fontFlags;
    font.FontData     = fontData;
    font.FontDataSize = dataSize;
    font.SubNum       = subNum;
    font.UniqueId     = NextFontId++;
    font.Type.map     = ~0U;
    font.Type.type    = ~0U;
    font.VectorHintingRange  = vectorHintingRange;
    font.RasterHintingRange  = rasterHintingRange;
    font.MaxVectorHintedSize = maxVectorHintedSize;
    font.MaxRasterHintedSize = maxRasterHintedSize;
    Fonts.PushBack(font);
}

//------------------------------------------------------------------------
Font* FontProviderPS3::CreateFont(const char* name, unsigned fontFlags)
{
    if (FontOK != CELL_OK)
        return 0;

#ifdef SF_ENABLE_THREADS
    Mutex::Locker lock(&LockMutex);
#endif

    // Mask flags to be safe.
    fontFlags &= Font::FF_CreateFont_Mask;
    fontFlags |= Font::FF_DeviceFont;

    unsigned i;
    for (i = 0; i < Fonts.GetSize(); ++i)
    {
        FontInfoPS3& font = Fonts[i];
        if (font.FontName.CompareNoCase(name) == 0 && 
           (font.FontFlags & Font::FF_Style_Mask) == (fontFlags & Font::FF_Style_Mask))
        {
            ExternalFontPS3* newFont = new ExternalFontPS3(this, Library, &font);
            if (newFont && !newFont->IsValid())
            {
                newFont->Release();
                return 0;
            }
            return newFont;
        }
    }
    return 0;
}

//------------------------------------------------------------------------
void FontProviderPS3::LoadFontNames(StringHash<String>& fontnames)
{
    for (unsigned i = 0; i < Fonts.GetSize(); ++i)
    {
        fontnames.Set(Fonts[i].FontName, Fonts[i].FontName);
    }
}

}}

