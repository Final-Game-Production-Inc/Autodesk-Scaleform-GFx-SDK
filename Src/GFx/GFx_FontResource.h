/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_FontResource.h
Content     :   Representation of SWF/GFX font data and resources.
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_FontResource_H
#define INC_SF_GFX_FontResource_H

#include "GFx/GFx_Types.h"
#include "Kernel/SF_Array.h"
//#include "Render/Render_Renderer.h"

//#include "Render/Render_ImageInfo.h"

#include "GFx/GFx_Player.h"
#include "GFx/GFx_Loader.h"

#include "GFx/GFx_ResourceHandle.h"
#include "GFx/GFx_ImageResource.h"

#include "GFx/GFx_Tags.h"

#include "Render/Render_Font.h"
#include "Render/Render_GlyphParam.h"

#include "GFx/GFx_FontCompactor.h"

namespace Scaleform { 

class File;

//using Render::ImageInfoBase;
//using Render::ImageInfo;

namespace GFx {

// ***** Declared Classes
class FontResource;
class FontData;
class TextureGlyphData;


// ***** External Classes
class MovieDataDef;
class ShapeDataBase;
class Stream;

class LoadProcess;

using Render::Font;
using Render::TextureGlyph;
using Render::ShapeDataInterface;
using Render::GlyphShape;

#ifndef GFX_FONT_CACHE_TEXTURE_SIZE_MINIMUM
#define GFX_FONT_CACHE_TEXTURE_SIZE_MINIMUM 32
#endif // GFX_FONT_CACHE_TEXTURE_SIZE_MINIMUM

// Class for Holding (cached) textured glyph info.
// class TextureGlyphInfo
// {
// public:
//     ResourcePtr<ImageResource>  pImage;
//     RectF                       UvBounds;
//     PointF                      UvOrigin;   // the origin of the glyph box, in uv coords
// 
//     // Sets image info to a resource.
//     void            SetImageResource(ImageResource *pres)        { pImage = pres; }
//     void            SetImageResource(const ResourceHandle& rh)   { pImage.SetFromHandle(rh); }
// 
//     // Return this texture glyph has an image resource assigned,
//     // and can thus be used for rendering.
//     bool            HasImageResource() const    { return !pImage.IsNull(); }
// };

//
// *** TextureGlyphData
//

// TextureGlyphData stores a collection of all textures and glyphs that
// can be used while rendering a font. It can either (1) be generated  automatically
// during SWF binding, or (2) be loaded from the GFX file. In the first case,
// the only pointer to data will be stored in FontResource, in the second case it
// will also be stored in FontData, so that it can be shared.
//
// The 'gfxexport' tool can cause TextureGlyphData to transition from
// dynamically generated to file-loaded state.


class TextureGlyphData : public RefCountBaseNTS<TextureGlyphData, StatMD_Fonts_Mem>
{
public:
    typedef FontPackParams::TextureConfig TextureConfig;

private:
    typedef HashLH<ResourceId, ResourcePtr<ImageResource>,
        FixedSizeHash<ResourceId>, StatMD_Fonts_Mem >  ImageResourceHash;

    // Describes texture packing settings used.
    TextureConfig           PackTextureConfig;

    // Texture glyphs structure; either built up by tha font packer
    // or loaded from GFX file by tag loaders.
    ArrayLH<TextureGlyph, StatMD_Fonts_Mem>   TextureGlyphs;
    // Generated textureID -> texture.
    ImageResourceHash       GlyphsTextures;

    // Set to 1 if info was loaded from GFX file, 0 if created dynamically.
    bool                    FileCreation;
public:

    TextureGlyphData(unsigned glyphCount = 0, bool isLoadedFromFile = false)
    {
        FileCreation = isLoadedFromFile;
        TextureGlyphs.Resize(glyphCount);
    }
    TextureGlyphData(const TextureGlyphData& orig);

    // Obtain/modify variables that control how glyphs are packed in textures.
    void    GetTextureConfig(TextureConfig* pconfig) const { *pconfig = PackTextureConfig; }
    void    SetTextureConfig(const TextureConfig& config)  { PackTextureConfig   = config; }
  
    float   GetTextureGlyphScale() const;

    bool    IsLoadedFromFile() const { return FileCreation;  }


    // *** Texture assignment / enumeration interfaces

    unsigned                GetTextureGlyphCount() const { return (unsigned)TextureGlyphs.GetSize(); }
    const TextureGlyph&     GetTextureGlyph(unsigned glyphIndex) const;

    void                    WipeTextureGlyphs();
    void                    AddTextureGlyph(unsigned glyphIndex, const TextureGlyph& glyph);

    void                    AddTexture(ResourceId textureId, ImageResource* pimageRes);
    void                    AddTexture(ResourceId textureId, const ResourceHandle &rh);

    unsigned                GetConfigTextureWidth()  const { return PackTextureConfig.TextureWidth;  }
    unsigned                GetConfigTextureHeight() const { return PackTextureConfig.TextureHeight; }
    int                     GetNominalSize() const { return PackTextureConfig.NominalSize; }

    struct TextureGlyphVisitor
    {
        virtual ~TextureGlyphVisitor() {}
        virtual void Visit(unsigned index, TextureGlyph* ptextureGlyph) = 0;
    };
    void                    VisitTextureGlyphs(TextureGlyphVisitor*);
    
    struct TexturesVisitor
    {
        virtual ~TexturesVisitor() {}
        virtual void Visit(ResourceId textureId, ImageResource* pimageRes) = 0;
    };
    void                    VisitTextures(TexturesVisitor*, ResourceBinding* pbinding);
};

// A base class for all fonts in GFx. This class knows about TextureGlyphData unlike Render::Font.
// class Font : public Render::Font
// {
// public:
//     Font(unsigned fontFlags = 0) : Render::Font(fontFlags) {}
// 
//     // TextureGlyphData access - only available if loaded from file.   
//     TextureGlyphData*   GetTextureGlyphData() const { return pTGData;  }
//     void                SetTextureGlyphData(TextureGlyphData* pdata)
//     {
//         SF_ASSERT(pdata->IsLoadedFromFile());
//         pTGData = pdata;
//     }
// 
//     // Return true if font contains texture glyphs.
//     bool        HasTextureGlyphs() const { return pTGData != NULL; }
// 
// protected:
    // Font texture glyphs data. This data is either generated dynamically based
    // on FontPackParams arguments or loaded from disc (in which case it is 
    // shared through pFont). Unlike FontData::pTextureGlyphData this TextureGlyphData is bound
    // to real textures and TextureGlyphs guaranteed to have pImage set.
//    Ptr<TextureGlyphData>   pTGData;
// };

//
// *** FontData
//

// FontData contains font data that is loaded from an SWF/GFX file.
// This data is associated with MovieDataDef and is thus shared among
// all of the MovieDefImpl instances that can potentially be created.
// This means that FontData object can not cache any instance-specific
// information, such as information about texture size and packing.
//  
// In general, FontData stores the following types of information
//  - Font name and attributes
//  - Kerning, Adance and layout variables
//  - Code points identifying characters in the font
//  - pTextureGlyphData loaded from exported file (*)
//
//  (*) pTextureGlyphData is special in that it contains texture size and
//  image information normally associated with a binding state. This information
//  is stored here ONLY if is loaded from that file; in that case it is shared
//  among all instances. If TextureGlyphData is instead generated dynamically
//  based on FontPackParams setting, as happens during SWF loading, it will only be
//  stored within FontResource instances (FontData::pTextureGlyphData will be null).


class FontData : public Font
{
public:
    FontData();
    FontData(const char* name, unsigned fontFlags);
    ~FontData();

    // *** GFX/SWF Loading methods.
    void            Read(LoadProcess* p, const TagInfo& tagInfo);
    void            ReadFontInfo(Stream* in, TagType tagType);
   
    
    // *** Font implementation
    

    virtual int             GetGlyphIndex(UInt16 code);
//     virtual bool            IsHintedVectorGlyph(unsigned, unsigned) const { return false; }
//     virtual bool            IsHintedRasterGlyph(unsigned, unsigned) const { return false; }
    virtual float           GetAdvance(unsigned glyphIndex) const;
    virtual float           GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const;

    virtual int             GetCharValue(unsigned glyphIndex) const;

    virtual unsigned        GetGlyphShapeCount() const      { return (unsigned)Glyphs.GetSize(); }

    virtual float           GetGlyphWidth(unsigned glyphIndex) const;
    virtual float           GetGlyphHeight(unsigned glyphIndex) const;
    virtual RectF&          GetGlyphBounds(unsigned glyphIndex, RectF* prect) const;

    // Slyph/State access.
    virtual const char*     GetName() const         { return Name; }

    // Only used for diagnostic purpose
    virtual String          GetCharRanges() const;

    virtual const Render::TextureGlyph* GetTextureGlyph(unsigned glyphIndex) const;
    virtual float                       GetTextureGlyphHeight() const;
    virtual void*           GetTextureGlyphData() const 
    { 
        return pTGData.GetPtr(); 
    }
    virtual void            SetTextureGlyphData(void* pvdata) 
    {
        TextureGlyphData* pdata = static_cast<TextureGlyphData*>(pvdata);
        pTGData = pdata;
    }

    virtual const Render::ShapeDataInterface* GetPermanentGlyphShape(unsigned glyphIndex) const;
     
    // *** Other APIs

    virtual bool            HasVectorOrRasterGlyphs() const { return CodeTable.GetSize() != 0; }    
    
private:
    void    ReadCodeTable(Stream* in);
    
    // General font information. 
    char*                   Name;

    // We store texture glyph data only if it comes from a file. Dynamically
    // created texture glyphs are accessible only through FontResource.
    Ptr<TextureGlyphData>   pTGData;
    
    // If Glyph shapes are loaded and not stripped, they are stored here.    
    ArrayLH<Ptr<ShapeDataBase>, StatMD_Fonts_Mem> Glyphs;


    // This table maps from Unicode character number to glyph index.
    // CodeTable[CharacterCode] = GlyphIndex
    typedef HashIdentityLH<UInt16, UInt16, StatMD_Fonts_Mem> CodeTableType;
    CodeTableType CodeTable;

    // Layout stuff.
    // Note that Leading, Advance and Descent are in the base class
    struct AdvanceEntry
    {
        float       Advance;
        SInt16      Left;   // offset by X-axis, in twips
        SInt16      Top;    // offset by Y-axis, in twips
        UInt16      Width;  // width, in twips
        UInt16      Height; // height, in twips
    };
    ArrayLH<AdvanceEntry, StatMD_Fonts_Mem>          AdvanceTable;
    
    struct KerningPair
    {           
        UInt16  Char0, Char1;
        bool    operator==(const KerningPair& k) const
            { return Char0 == k.Char0 && Char1 == k.Char1; }
    };
    HashLH<KerningPair, float, FixedSizeHash<KerningPair>, StatMD_Fonts_Mem>  KerningPairs;

    // @@ we don't seem to use this thing at all, so don't bother keeping it.
    // Array<Render::Renderer::Rect> BoundsTable;    // @@ this thing should be optional.
};


#ifdef GFX_ENABLE_FONT_COMPACTOR
//
// *** FontDataCompactedSwf
//
// This is used for DefineFont2 and DefineFont3 tags and when FontCompactorParams 
// state is set to the loader. It reads SWF font data and then convert it into more compact
// form.
class FontDataCompactedSwf : public Font
{
public:
    typedef FontCompactor::ContainerType ContainerType;
    typedef CompactedFont<ContainerType> CompactedFontType;

    FontDataCompactedSwf();
    ~FontDataCompactedSwf();

    // *** GFX/SWF Loading methods.
    void            Read(LoadProcess* p, const TagInfo& tagInfo);

    // *** Font implementation

    // Glyph/State access.
    virtual const char*     GetName() const;

    int                     GetGlyphIndex(UInt16 code);
    float                   GetAdvance(unsigned glyphIndex) const;
    float                   GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const;
    float                   GetGlyphWidth(unsigned glyphIndex) const;
    float                   GetGlyphHeight(unsigned glyphIndex) const;
    RectF&                  GetGlyphBounds(unsigned glyphIndex, RectF* prect) const;
    const ShapeDataInterface* GetPermanentGlyphShape(unsigned) const { return 0; }
    bool                    GetTemporaryGlyphShape(unsigned glyphIndex, unsigned hintedSize, GlyphShape* shape);

    int                     GetCharValue(unsigned glyphIndex) const;
    unsigned                GetGlyphShapeCount() const;


    // Only used for diagnostic purpose
    virtual String          GetCharRanges() const;

    bool                    HasVectorOrRasterGlyphs() const;

    const ContainerType& GetContainer () const {return Container;}
    const CompactedFontType& GetCompactedFont() const { return CompactedFontValue; }

private:

    // We store texture glyph data only if it comes from a file. Dynamically
    // created texture glyphs are accessible only through FontResource.
    //Ptr<TextureGlyphData>   pTextureGlyphData;
 
    ContainerType           Container;
    CompactedFontType       CompactedFontValue;
    unsigned                NumGlyphs;
};
#endif //#ifdef GFX_ENABLE_FONT_COMPACTOR

#ifdef GFX_ENABLE_COMPACTED_FONTS
//
// *** FontDataCompactedGfx

//
// This is used for DefineCompactedFont tag. This tag is created
// by gfxexport when font is converted into a compact form.
class FontDataCompactedGfx : public Font
{
public:
    typedef ArrayUnsafeLH_POD<UByte, StatMD_Fonts_Mem> ContainerType;
    typedef CompactedFont<ContainerType> CompactedFontType;

    FontDataCompactedGfx();
    ~FontDataCompactedGfx();

    // *** GFX Loading methods.
    void            Read(LoadProcess* p, const TagInfo& tagInfo);

    // *** Font implementation

    // Glyph/State access.
    virtual const char*     GetName() const;
    int                     GetGlyphIndex(UInt16 code);
    float                   GetAdvance(unsigned glyphIndex) const;
    float                   GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const;
    float                   GetGlyphWidth(unsigned glyphIndex) const;
    float                   GetGlyphHeight(unsigned glyphIndex) const;
    RectF&                  GetGlyphBounds(unsigned glyphIndex, RectF* prect) const;
    const ShapeDataInterface* GetPermanentGlyphShape(unsigned) const { return 0; }
    bool                    GetTemporaryGlyphShape(unsigned glyphIndex, unsigned hintedSize, GlyphShape* shape);

    int                     GetCharValue(unsigned glyphIndex) const;

    unsigned                GetGlyphShapeCount() const;

    // Only used for diagnostic purpose
    virtual String          GetCharRanges() const;

    bool                    HasVectorOrRasterGlyphs() const;

    const ContainerType& GetContainer () const {return Container;}
    const CompactedFontType& GetCompactedFont() const { return CompactedFontValue; }

private:

    // We store texture glyph data only if it comes from a file. Dynamically
    // created texture glyphs are accessible only through FontResource.
    //Ptr<TextureGlyphData>  pTextureGlyphData;

    ContainerType     Container;
    CompactedFontType CompactedFontValue;
};

#endif //#ifdef GFX_ENABLE_COMPACTED_FONTS

//
// *** FontResource
//
// FontResource is a font-instance resource that delegates a lot of its functionality
// to FontData and GFxTextureData.


class FontDataBound : public Font
{
public:
    FontDataBound(Font* pfont, ResourceBinding* pbinding);

    // Get the font name in UTF-8 string format.
    virtual const char* GetName() const { return pFont->GetName(); }

    // Map a UTF-16 code to the glyph index. In case the glyph is not available
    // returns -1. The function is non-const, but it can be thread-safe and read-only in case
    // the glyph table is permanently available. In other cases, such as external font providers
    // it can modify the glyph table on demand, but must be synchronized and thread-safe.
    virtual int    GetGlyphIndex(UInt16 code) { return pFont->GetGlyphIndex(code); }

    virtual float  GetAdvance(unsigned glyphIndex) const { return pFont->GetAdvance(glyphIndex); }
    virtual float  GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const 
    { 
        return pFont->GetKerningAdjustment(lastCode, thisCode); 
    }
    virtual float  GetGlyphWidth(unsigned glyphIndex) const { return pFont->GetGlyphWidth(glyphIndex); }
    virtual float  GetGlyphHeight(unsigned glyphIndex) const { return pFont->GetGlyphHeight(glyphIndex); }
    virtual RectF& GetGlyphBounds(unsigned glyphIndex, RectF* prect) const 
    { 
        return pFont->GetGlyphBounds(glyphIndex, prect); 
    }

    // The function returns true in case native hinting is available for glyphSize.
    virtual bool IsHintedVectorGlyph(unsigned glyphIndex, unsigned hintedSize) const
    {
        return pFont->IsHintedVectorGlyph(glyphIndex, hintedSize);
    }

    // The function returns true in case a raster with native hinting is available for glyphSize.
    virtual bool IsHintedRasterGlyph(unsigned glyphIndex, unsigned hintedSize) const
    {
        return pFont->IsHintedRasterGlyph(glyphIndex, hintedSize);
    }

    // Some fonts may have pre-rasterized and pre-packed raster glyphs in textures (static font cache).
    // If they exist, they will be used instead of vector pipelines.
    virtual const TextureGlyph* GetTextureGlyph(unsigned glyphIndex) const;

    // Returns a pointer to TextureGlyphData, if exists.true if font contains texture glyphs.
    virtual void*   GetTextureGlyphData() const { return pTGData; }

    // Returns nominal texture glyph nominal height
    virtual float GetTextureGlyphHeight() const { return pFont->GetTextureGlyphHeight(); }

    // Function is used in case all the glyphs are permanently available, while the font is alive.
    // In case the shapes are not persistent, the function must return 0. Then the shapes will be 
    // obtained by GetTemporaryGlyphShape and cached.
    virtual const ShapeDataInterface* GetPermanentGlyphShape(unsigned glyphIndex) const
    {
        return pFont->GetPermanentGlyphShape(glyphIndex);
    }

    // The function is used when GetPermanentGlyphShape returns 0. Mostly used in external 
    // font providers. This interface allows the font provider to do without caching, as the
    // shapes will be cached in the font cache system. In case GetPermanentGlyphShape works
    // GetTemporaryGlyphShape is not necessary to override.
    virtual bool GetTemporaryGlyphShape(unsigned glyphIndex, unsigned hintedSize, Render::GlyphShape* shape)
    {
        return pFont->GetTemporaryGlyphShape(glyphIndex, hintedSize, shape);
    }

    // Some fonts may provide raster glyphs, pre-rasterized, or rasterized on demand. The feature 
    // is especially important for CJK fonts at small font sizes. In case the font does not provide
    // raster glyphs, it's not necessary to override the function.
    virtual bool GetGlyphRaster(unsigned glyphIndex, unsigned hintedSize, Render::GlyphRaster* raster)
    {
        return pFont->GetGlyphRaster(glyphIndex, hintedSize, raster);
    }

    // Returns the ucs2 char value associated with a glyph index
    // This may need to change in the future if composite glyphs such
    // as presentation forms are supported (return a string 
    // in such a cases). It is used for taking snapshots of static
    // textfields, and therefore is overridden by the following
    // derived classes:
    // GFxFontData, GFxFontDataCompactedGFx, GFxFontDataCompactedSwf
    virtual int         GetCharValue(unsigned glyphIndex) const 
    {
        return pFont->GetCharValue(glyphIndex);
    }

    // Returns the number of glyph shapes, mostly used for font packing.
    // Can return 0 for system fonts, where the # of shapes is not known ahead of time.
    virtual unsigned    GetGlyphShapeCount() const { return pFont->GetGlyphShapeCount(); }

    virtual bool        HasVectorOrRasterGlyphs() const { return pFont->HasVectorOrRasterGlyphs(); }

    // Used only for diagnostic purpose
    virtual String      GetCharRanges() const { return pFont->GetCharRanges(); }

    // Default sizes reported for non-existing glyph indices.
    virtual float       GetNominalGlyphWidth() const  { return pFont->GetNominalGlyphWidth(); }
    virtual float       GetNominalGlyphHeight() const { return pFont->GetNominalGlyphHeight(); }

protected:

    Ptr<Font>           pFont;

    // Font texture glyphs data. This data is either generated dynamically based
    // on FontPackParams arguments or loaded from disc (in which case it is 
    // shared through pFont). Unlike FontData::pTextureGlyphData this TextureGlyphData is bound
    // to real textures and TextureGlyphs guaranteed to have pImage set.
    Ptr<TextureGlyphData>   pTGData;
};

class FontResource : public Resource
{
public:
    typedef Render::Font::FontFlags FontFlags;
    
    // We store fonts binding within font resource for convenient access; this
    // binding is used to look up image resources from TextureGlyphData. It is ok
    // because separate FontResource is always created for every MovieDefImpl.
    // Note that the pbinding can also be null for system fonts.
    FontResource(Font *pfont, ResourceBinding* pbinding);
    // For system provider fonts we pass a key.
    FontResource(Font *pfont, const ResourceKey& key);
    ~FontResource();

    Font*               GetFont()    const { return pFont; }
    ResourceBinding*    GetBinding() const { return pBinding; }

    
    // *** Handler List support.

//     // Handlers are used to allow font cache manager to
//     // detect when fonts are released.        
//     class DisposeHandler
//     {
//     public:
//         virtual ~DisposeHandler() {}
//         virtual void OnDispose(FontResource*) = 0;
//     };
// 
//     void                    AddDisposeHandler(DisposeHandler* h);
//     void                    RemoveDisposeHandler(DisposeHandler* h);


    // *** Delegates to Font

    // implement FontData.States = *pFont;

    bool                    MatchFont(const char* name, unsigned matchFlags) const { return pFont->MatchFont(name, matchFlags); }
    bool                    MatchFontFlags(unsigned matchFlags) const              { return pFont->MatchFontFlags(matchFlags); }
    
    inline const char*      GetName() const                     { return pFont->GetName(); }
    inline unsigned         GetGlyphShapeCount() const          { return pFont->GetGlyphShapeCount(); }

    //TextureGlyphData*       GetTextureGlyphData() const         
        //{ return pTGData.GetPtr(); }
    //ShapeDataBase*              GetGlyphShape(unsigned glyphIndex, unsigned glyphSize) const 
    //{ 
    //    return pFont->GetGlyphShape(glyphIndex, glyphSize); 
    //}
    //GlyphRaster*            GetGlyphRaster(unsigned glyphIndex, unsigned glyphSize) const 
    //{
    //    return pFont->GetGlyphRaster(glyphIndex, glyphSize); 
    //}

    int                     GetGlyphIndex(UInt16 code) const     { return pFont->GetGlyphIndex(code); }
    bool                    IsHintedVectorGlyph(unsigned glyphIndex, unsigned glyphSize) const { return pFont->IsHintedVectorGlyph(glyphIndex, glyphSize); } 
    bool                    IsHintedRasterGlyph(unsigned glyphIndex, unsigned glyphSize) const { return pFont->IsHintedRasterGlyph(glyphIndex, glyphSize); }
    float                   GetAdvance(unsigned glyphIndex) const    { return pFont->GetAdvance(glyphIndex); }
    float                   GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const { return pFont->GetKerningAdjustment(lastCode, thisCode);  }
    float                   GetGlyphWidth(unsigned glyphIndex) const  { return pFont->GetGlyphWidth(glyphIndex); }
    float                   GetGlyphHeight(unsigned glyphIndex) const { return pFont->GetGlyphHeight(glyphIndex);  }
    RectF&                  GetGlyphBounds(unsigned glyphIndex, RectF* prect) const { return pFont->GetGlyphBounds(glyphIndex, prect);  }
    float                   GetLeading() const                   { return pFont->GetLeading(); }
    float                   GetDescent() const                   { return pFont->GetDescent(); }
    float                   GetAscent() const                    { return pFont->GetAscent(); }
 
    inline unsigned         GetFontFlags() const                { return pFont->GetFontFlags(); }
    inline unsigned         GetFontStyleFlags() const           { return pFont->GetFontStyleFlags(); }
    inline unsigned         GetCreateFontFlags() const          { return pFont->GetCreateFontFlags(); }
    inline bool             IsItalic() const                    { return pFont->IsItalic(); } 
    inline bool             IsBold() const                      { return pFont->IsBold(); } 
    inline bool             IsDeviceFont() const                { return pFont->IsDeviceFont(); } 
    inline bool             HasNativeHinting() const            { return pFont->HasNativeHinting(); }
 
    inline bool             AreUnicodeChars() const             { return pFont->AreUnicodeChars(); } 
    inline FontFlags        GetCodePage() const                 { return pFont->GetCodePage(); } 
    inline bool             GlyphShapesStripped() const         { return pFont->GlyphShapesStripped(); }
    inline bool             HasLayout() const                   { return pFont->HasLayout(); }     
    inline bool             AreWideCodes() const                { return pFont->AreWideCodes(); }     
    inline bool             IsPixelAligned() const              { return pFont->IsPixelAligned(); }

    inline UInt16 GetLowerCaseTop(Log* log)
    { 
        if (LowerCaseTop == 0)
            calcLowerUpperTop(log);
        return (LowerCaseTop <= 0) ? 0 : LowerCaseTop; 
    }
    inline UInt16 GetUpperCaseTop(Log* log) 
    { 
        if (UpperCaseTop == 0)
            calcLowerUpperTop(log);
        return (UpperCaseTop <= 0) ? 0 : UpperCaseTop; 
    }

    // *** Delegates to TextureGlyphData

    //typedef FontPackParams::TextureConfig TextureConfig;

    // TBD: Necessary?

    //inline void GetTextureConfig(TextureConfig* pconfig) const { if (pTGData) pTGData->GetTextureConfig(pconfig); }

    // TextureGlyphData access
    //void                    SetTextureGlyphData(TextureGlyphData* pdata) { pTGData = pdata; }


    // *** Resource implementation
    
    virtual unsigned        GetResourceTypeCode() const     { return MakeTypeCode(RT_Font); }
    virtual ResourceKey     GetKey()                        { return FontKey; }

    // Helper function used to lookup and/or create a font resource from provider.
    static FontResource*    CreateFontResource(const char* pname, unsigned fontFlags,
                                               FontProvider* pprovider, ResourceWeakLib *plib);
    // Creates a system font resource key.
    static ResourceKey      CreateFontResourceKey(const char* pname, unsigned fontFlags,
                                                  FontProvider* pfontProvider);
    // Resolves TextureGlyphData (images), if any. Called from BindNextFrame.
    void ResolveTextureGlyphs();
private:
    UInt16 calcTopBound(UInt16 code);
    void   calcLowerUpperTop(Log* log);

    Ptr<Font>               pFont;

    // Font texture glyphs data. This data is either generated dynamically based
    // on FontPackParams arguments or loaded from disc (in which case it is 
    // shared through pFont).
//    Ptr<TextureGlyphData>   pTGData;

    // Pointer to font binding that should be used with embedded fonts; null for
    // system fonts. This pointer would be different in case of imports. It is the 
    // responsibility of the importing file (for direct imports) or font manager 
    // (for font provider lookups) to hold the MovieDefImpl for this binding.    
    ResourceBinding*        pBinding;

    // Font key - we store these only for provider-obtained fonts.
    ResourceKey             FontKey;
   
    // Handlers list data structures. 
//     bool                    HandlerArrayFlag;
//     union {
//         DisposeHandler*             pHandler;
//         Array<DisposeHandler*> *    pHandlerArray;  
//     };

    // Values used for pixel-grid fitting. Defined as the
    // top Y coordinate for 'z' and 'Z' respectively
    SInt16 LowerCaseTop;
    SInt16 UpperCaseTop;
};

}} // Scaleform::GFx

#endif 
