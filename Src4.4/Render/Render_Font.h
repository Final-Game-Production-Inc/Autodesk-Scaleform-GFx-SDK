/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Font.h
Content     :   Abstract base class for system and built-in fonts.
Created     :   6/18/2007
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Font_H
#define INC_SF_Render_Font_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_String.h"
#include "Kernel/SF_StringHash.h"
#include "Kernel/SF_List.h"
#include "Render_Types2D.h"
#include "Render_ShapeDataPacked.h"
#include "Render_Stats.h"

namespace Scaleform { namespace Render {


class FontCacheHandleManager;
class FontCacheHandle;
class GlyphCache;
struct TextureGlyph;

//------------------------------------------------------------------------
class GlyphRaster : public RefCountBaseNTS<GlyphRaster, StatRender_Font_Mem>
{
public:
    GlyphRaster() : Width(0), Height(0), OriginX(0), OriginY(0), HintedSize(0) {}

    ArrayLH_POD<UByte, StatRender_Font_Mem> Raster;
    unsigned    Width, Height;
    int         OriginX, OriginY;
    unsigned    HintedSize;
};



//------------------------------------------------------------------------
class GlyphShape : public ShapeDataPacked<ArrayLH_POD<UByte> >
{
    typedef ArrayLH_POD<UByte> ContainerType;
public:
    GlyphShape() : ShapeDataPacked<ArrayLH_POD<UByte> > (&Data, 0, 1.0f), HintedSize(0) {}

    void         Clear()         { Data.Clear(); }
    virtual bool IsEmpty() const { return Data.GetSize() == 0; }

    void     SetHintedSize(unsigned size) { HintedSize = size; }
    unsigned GetHintedSize() const { return HintedSize; }

private:
    ContainerType   Data;
    unsigned        HintedSize;
    RectF           Bounds;
};


//------------------------------------------------------------------------
// FontCacheHandle maintained in GlyphCache to
//  - Find fonts efficiently based on provider pointer
//  - Inform cache system when provider is destroyed
//    (needed to free up cache space and ensure there are no
//     same-address pointer matches in case of re-allocation).
//
// This class is Partially thread safe as follows:
//  - Its constructor and destructor can be called on any thread.
//    Destructor notifies FontCacheHandleManager in thread-safe manager.
// Besides destruction, this object is only accessed on the Render Thread:
//  - pFont can be read without a lock by RenderThread only,
//    it is always modified during GlyphCache lock.
//  - Render thread will call releaseCache_NoNotify() when it
//    swaps out the associated Font.
class FontCacheHandleRef
{
    friend class FontCacheHandleManager;
    friend class FontCacheHandle;

    AtomicPtr<FontCacheHandleManager> pManager;
    FontCacheHandle* volatile         pFontHandle;
    void releaseFont();
    void releaseFont_NoNotify();

public:
    FontCacheHandleRef() : pFontHandle(0)
    { }
    ~FontCacheHandleRef()
    { releaseFont(); }
};

//------------------------------------------------------------------------
class Font : public RefCountBase<Font, StatRender_Font_Mem> // TO DO: StatID
{
    friend class FontCacheHandleManager;

public:
    Font(unsigned fontFlags = 0)
        : Ascent(0), Descent(0), Leading(0), Flags(fontFlags),
          LowerCaseTop(0), UpperCaseTop(0)

    {
    }
    Font(const Font& f) : Ascent(f.Ascent), Descent(f.Descent),
        Leading(f.Leading), Flags(f.Flags), LowerCaseTop(f.LowerCaseTop),
        UpperCaseTop(f.UpperCaseTop)
    {
    }
    virtual ~Font() {}

    enum FontFlags
    {
        // General font style flags.
        FF_Italic               = 0x0001,
        FF_Bold                 = 0x0002,
        FF_BoldItalic           = FF_Bold | FF_Italic,

        // These flag bits are used for matching a font. This mask is used
        // for both font searching and hashing.
        FF_Style_Mask           = FF_BoldItalic,

        // This flag is set if the font is being requested from a system device;
        // this can be set by the text field if it prefers system glyphs over
        // the SWF-embedded glyphs.
        FF_DeviceFont           = 0x0010,

        // Native Hinting can be used with the external font providers, 
        // such as Win32 or FreeType. This flag is set in the external 
        // font constructor.
        FF_NativeHinting        = 0x0020,

        // The font should be resolved through FontLib or FontProvider
        FF_NotResolved          = 0x0040,

        // Font has raster glyphs only
        FF_RasterOnly           = 0x0080,

        // Code page bits. Fonts supported right now are all Unicode,
        // but SWFs before version 6 allowed code pages.
        FF_CodePage_Mask        = 0x0300,
        FF_CodePage_Unicode     = 0x0000,
        FF_CodePage_Ansi        = 0x0100,
        FF_CodePage_ShiftJis    = 0x0200,

        // Mask passed to FontProvider::CreateFont. 
        FF_CreateFont_Mask     = FF_Style_Mask | FF_CodePage_Mask,
       
        // Flash - specific attributes.
        FF_GlyphShapesStripped  = 0x1000,
        FF_HasLayout            = 0x2000,
        FF_WideCodes            = 0x4000,
        FF_PixelAligned         = 0x8000,  // set to 1 for so called "bitmap fonts"

        // Texture Fonts
        FF_DistanceFieldAlpha   = 0x10000
    };

    // Used in external font providers
    enum NativeHintingRange
    {
        DontHint, // Don't apply native hinting
        HintCJK,  // Hint CJK only
        HintAll   // Hint all glyphs
    };

    // Used in external font providers
    struct NativeHintingType
    {
        String              Typeface;
        NativeHintingRange  RasterRange;
        NativeHintingRange  VectorRange;
        unsigned            MaxRasterHintedSize;
        unsigned            MaxVectorHintedSize;
    };

    static bool IsMissingGlyph(unsigned glyphIndex) { return (glyphIndex & 0xFFFF) == 0xFFFF; }

    // Matches font name and type for substitution purposes.
    bool MatchSubstituteFont(Font *fd) const
    {
        return MatchFont(fd->GetName(), fd->GetCreateFontFlags());
    }

    // Get the font name in UTF-8 string format.
    virtual const char* GetName() const = 0;

    // Map a UTF-16 code to the glyph index. In case the glyph is not available
    // returns -1. The function is non-const, but it can be thread-safe and read-only in case
    // the glyph table is permanently available. In other cases, such as external font providers
    // it can modify the glyph table on demand, but must be synchronized and thread-safe.
    virtual int    GetGlyphIndex(UInt16 code) = 0;

    virtual float  GetAdvance(unsigned glyphIndex) const = 0;
    virtual float  GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const = 0;
    virtual float  GetGlyphWidth(unsigned glyphIndex) const = 0;
    virtual float  GetGlyphHeight(unsigned glyphIndex) const = 0;
    virtual RectF& GetGlyphBounds(unsigned glyphIndex, RectF* prect) const = 0;

    // The function returns true in case native hinting is available for glyphSize.
    virtual bool IsHintedVectorGlyph(unsigned glyphIndex, unsigned hintedSize) const
    {
        SF_UNUSED2(glyphIndex, hintedSize);
        return false;
    }

    // The function returns true in case a raster with native hinting is available for glyphSize.
    virtual bool IsHintedRasterGlyph(unsigned glyphIndex, unsigned hintedSize) const
    {
        SF_UNUSED2(glyphIndex, hintedSize);
        return false;
    }

    // Some fonts may have pre-rasterized and pre-packed raster glyphs in textures (static font cache).
    // If they exist, they will be used instead of vector pipelines.
    virtual const TextureGlyph* GetTextureGlyph(unsigned glyphIndex) const { SF_UNUSED(glyphIndex); return 0; }

    // Getter/setter for abstract texture glyph data
    virtual void*       GetTextureGlyphData() const { return NULL; }
    virtual void        SetTextureGlyphData(void*) {}

    // Returns nominal texture glyph nominal height
    virtual float GetTextureGlyphHeight() const { return 0; }
    
    // Function is used in case all the glyphs are permanently available, while the font is alive.
    // In case the shapes are not persistent, the function must return 0. Then the shapes will be 
    // obtained by GetTemporaryGlyphShape and cached.
    virtual const ShapeDataInterface* GetPermanentGlyphShape(unsigned glyphIndex) const = 0;

    // The function is used when GetPermanentGlyphShape returns 0. Mostly used in external 
    // font providers. This interface allows the font provider to do without caching, as the
    // shapes will be cached in the font cache system. In case GetPermanentGlyphShape works
    // GetTemporaryGlyphShape is not necessary to override.
    virtual bool GetTemporaryGlyphShape(unsigned glyphIndex, unsigned hintedSize, GlyphShape* shape)
    {
        SF_UNUSED3(glyphIndex, hintedSize, shape);
        return false;
    }

    // Some fonts may provide raster glyphs, pre-rasterized, or rasterized on demand. The feature 
    // is especially important for CJK fonts at small font sizes. In case the font does not provide
    // raster glyphs, it's not necessary to override the function.
    virtual bool GetGlyphRaster(unsigned glyphIndex, unsigned hintedSize, GlyphRaster* raster)
    {
        SF_UNUSED3(glyphIndex, hintedSize, raster);
        return false;
    }

    // Returns the ucs2 char value associated with a glyph index
    // This may need to change in the future if composite glyphs such
    // as presentation forms are supported (return a string 
    // in such a cases). It is used for taking snapshots of static
    // textfields, and therefore is overridden by the following
    // derived classes:
    // GFxFontData, GFxFontDataCompactedGFx, GFxFontDataCompactedSwf
    virtual int         GetCharValue(unsigned glyphIndex) const { SF_UNUSED(glyphIndex); return -1; }

    // Returns the number of glyph shapes, mostly used for font packing.
    // Can return 0 for system fonts, where the # of shapes is not known ahead of time.
    virtual unsigned    GetGlyphShapeCount() const { return 0; }

    virtual bool        HasVectorOrRasterGlyphs() const { return true; }

    // Used only for diagnostic purpose
    virtual String      GetCharRanges() const { return ""; }

    // Default sizes reported for non-existing glyph indices.
    virtual float       GetNominalGlyphWidth() const  { return 512.0f; }
    virtual float       GetNominalGlyphHeight() const { return 1024.0f; }

    // Return true if font contains texture glyphs.
    bool        HasTextureGlyphs() const { return GetTextureGlyphData() != NULL; }
    float       GetLeading() const  { return Leading; }
    float       GetDescent() const  { return Descent; }
    float       GetAscent()  const  { return Ascent;  }
    
    unsigned    GetFontFlags() const { return Flags; }
    unsigned    GetFontStyleFlags() const { return Flags & FF_Style_Mask; }
    unsigned    GetCreateFontFlags() const { return Flags & FF_CreateFont_Mask; }
    // Flag query.
    bool        IsItalic() const            { return (Flags & FF_Italic) != 0; } 
    bool        IsBold() const              { return (Flags & FF_Bold) != 0; } 
    bool        IsDeviceFont() const        { return (Flags & FF_DeviceFont) != 0; } 
    bool        HasNativeHinting() const    { return (Flags & FF_NativeHinting) != 0; }
    bool        IsRasterOnly() const        { return (Flags & FF_RasterOnly) != 0; }

    FontFlags   GetCodePage() const         { return (FontFlags)(Flags & FF_CodePage_Mask); }
    bool        AreUnicodeChars() const     { return GetCodePage() == FF_CodePage_Unicode; }
    bool        GlyphShapesStripped() const { return (Flags & FF_GlyphShapesStripped) != 0; }
    bool        HasLayout() const           { return (Flags & FF_HasLayout) != 0; }
    bool        AreWideCodes() const        { return (Flags & FF_WideCodes) != 0; }
    bool        IsPixelAligned() const      { return (Flags & FF_PixelAligned) != 0; }

    bool        IsResolved() const          { return (Flags & FF_NotResolved) == 0; }
    void        SetNotResolvedFlag()        { Flags |= FF_NotResolved; }

    // MatchFont includes a particular heuristic for search based on a font name, helpful
    // for handling Flash DeviceFont flag in text fields:
    //  - If FF_DeviceFont is NOT specified all searches are done based on FF_StyleFont_Mask,
    //    and both regular and device fonts can be returned.
    //  - If FF_DeviceFont IS specified, only device fonts will be matched.
    // The same rule applies to code page - it is considered only if specified as non-unicode.
    static bool SF_STDCALL MatchFontFlags_Static(unsigned fontFlags, unsigned matchFlags)
    {
        unsigned matchMask= (matchFlags & FF_DeviceFont) | ((matchFlags & FF_CodePage_Mask) ? FF_CodePage_Mask : 0);
        unsigned ourFlags = fontFlags & (FF_Style_Mask | matchMask);
        unsigned flags    = matchFlags & (FF_Style_Mask | FF_DeviceFont | FF_CodePage_Mask);
        return (ourFlags == flags);
    }

    // Returns true if the character is a CJK (Chinese, Japanese, Korean)
    // The function is used for strong native hinting with device fonts.
    bool IsCJK(UInt16 code) const;

    bool MatchFontFlags(unsigned matchFlags) const
    {
        return MatchFontFlags_Static(GetFontFlags(), matchFlags);
    }

    bool MatchFont(const char* name, unsigned matchFlags) const
    {
        return (MatchFontFlags(matchFlags) && !String::CompareNoCase(GetName(), name));
    }

    inline UInt16 GetLowerCaseTop(GlyphCache* log)
    { 
        if (LowerCaseTop == 0)
            calcLowerUpperTop(log);
        return (LowerCaseTop <= 0) ? 0 : LowerCaseTop; 
    }
    inline UInt16 GetUpperCaseTop(GlyphCache* log) 
    { 
        if (UpperCaseTop == 0)
            calcLowerUpperTop(log);
        return (UpperCaseTop <= 0) ? 0 : UpperCaseTop; 
    }

protected:
    UInt16 calcTopBound(UInt16 code);
    void   calcLowerUpperTop(GlyphCache* log);

    void SetFontMetrics(float leading, float ascent, float descent)
    {
        Leading = leading;
        Ascent  = ascent;
        Descent = descent;
    }

    void    SetItalic(bool v = true)        { (v) ? Flags |= FF_Italic : Flags &= ~FF_Italic; }
    void    SetBold(bool v = true)          { (v) ? Flags |= FF_Bold : Flags &= ~FF_Bold; }
    void    SetCodePage(FontFlags codePage) { Flags = (Flags & ~FF_CodePage_Mask) | (codePage & FF_CodePage_Mask); }
    void    SetGlyphShapesStripped()        { Flags |= FF_GlyphShapesStripped; }
    void    SetHasLayout(bool v = true)     { (v) ? Flags |= FF_HasLayout : Flags &= ~FF_HasLayout; }
    void    SetWideCodes(bool v = true)     { (v) ? Flags |= FF_WideCodes : Flags &= ~FF_WideCodes; }
    void    SetPixelAligned(bool v = true)  { (v) ? Flags |= FF_PixelAligned : Flags &= ~FF_PixelAligned; }


    float    Ascent;
    float    Descent;
    float    Leading;    
    unsigned Flags;   // Described by FontFlags enum.  
    SInt16   LowerCaseTop, UpperCaseTop;
    FontCacheHandleRef  hRef;
};





// TO DO: Conform with "Loader Stats"
//------------------------------------------------------------------------
class FontProvider : public RefCountBase<FontProvider, StatRender_Font_Mem>
{
public:
    virtual ~FontProvider() {}
    
    // fontFlags style as described by Font::FontFlags
    virtual Font* CreateFont(const char* name, unsigned fontFlags) = 0;

    // EnumerateFonts interface...
    virtual void LoadFontNames(StringHash<String>& fontnames) = 0;
};








}} // Scaleform::GFx

#endif

