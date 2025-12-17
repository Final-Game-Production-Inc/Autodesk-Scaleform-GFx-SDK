/**************************************************************************

Filename    :   GFx_FontGlyphPacker.h
Content     :   FontGlyphPacker implementation declaration
Created     :   6/14/2007
Authors     :   Maxim Shemanarev, Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_FontGlyphPacker_H
#define INC_SF_GFX_FontGlyphPacker_H

#include "Kernel/SF_Array.h"
#include "Render/Render_Types2D.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_FontResource.h"

#ifdef SF_RENDER_ENABLE_GLYPH_PACKER
//#include "Render/Render_CompoundShape.h"
#include "Render/Render_RectPacker.h"
#include "Render/Render_Rasterizer.h"
#include "GFx/GFx_Shape.h"

namespace Scaleform { namespace GFx {

//------------------------------------------------------------------------
bool    IsEqualGeometry(const Render::ShapeDataInterface& a, const Render::ShapeDataInterface& b);
UInt32  ComputeGeometryHash(const Render::ShapeDataInterface& sh);


//------------------------------------------------------------------------
class FontGlyphPacker : public RefCountBaseNTS<FontGlyphPacker, Stat_Default_Mem>
{
    // This is for keeping track of our rendered glyphs, before
    // packing them into textures and registering with the FontResource.
    struct GlyphInfo
    {
        FontResource*   pFont;
        unsigned        GlyphIndex;
        unsigned        GlyphReuse;
        unsigned        TextureIdx;
        RectF           Bounds;
        Render::PointF  Origin;
    };

    struct GlyphGeometryKey
    {
        // Make sure the member variables stay with no padding
        // Padding breaks the hash function that uses this struct as a key
        const FontResource*                 pFont;
        const Render::ShapeDataInterface*   pShape;
        UInt32                              Hash;

        GlyphGeometryKey(): pFont(0), pShape(0), Hash(0) {}
        GlyphGeometryKey(const FontResource* font, 
                         const Render::ShapeDataInterface* shape, 
                         UInt32 hash): 
            pFont(font), pShape(shape), Hash(hash) {}
        UPInt operator()(const GlyphGeometryKey& data) const
        {
            return (UPInt)data.Hash ^ 
                  ((UPInt)data.pFont >> 6) ^ 
                   (UPInt)data.pFont;
        }
        bool operator== (const GlyphGeometryKey& cmpWith) const 
        { 
            return pFont == cmpWith.pFont && 
                   IsEqualGeometry(*pShape, *cmpWith.pShape);
        }
    };

public:
    FontGlyphPacker(FontPackParams* params,
                       ImageCreator *pimageCreator,
                       Log* plog,
                       ResourceId* ptextureIdGen,
                       MemoryHeap* fontHeap,
                       bool threadedLoading);
   ~FontGlyphPacker();

    void GenerateFontBitmaps(const Array<FontResource*>& fonts);


private:
    // Prohibit Copying
    FontGlyphPacker(const FontGlyphPacker&);
    const FontGlyphPacker& operator = (const FontGlyphPacker&);

    void generateGlyphInfo(Array<GlyphInfo>* glyphs, FontResource* f);
    unsigned packGlyphRects(Array<GlyphInfo>* glyphs, 
                        unsigned start, unsigned end, unsigned texIdx);
    unsigned packGlyphRects(Array<GlyphInfo>* glyphs);
    void rasterizeGlyph(Render::RawImage* texImage, GlyphInfo* gi);
    void generateTextures(Array<GlyphInfo>* glyphs, unsigned numTextures);

    typedef HashLH<GlyphGeometryKey, unsigned, GlyphGeometryKey,  Stat_Default_Mem> GlyphGeometryHashType;

    FontPackParams*                 pFontPackParams;
    // Texture glyph configuration cached from FontPackParams.
    FontPackParams::TextureConfig   PackTextureConfig;

    // Texture id generator, specified externally.
    ResourceId*                     pTextureIdGen;
    Ptr<ImageCreator>               pImageCreator;
    Ptr<Log>                        pLog;
    MemoryHeap*                     pFontHeap;

    Render::RectPacker              Packer;
    Render::Rasterizer              Ras;
    GlyphGeometryHashType           GlyphGeometryHash;
    bool                            ThreadedLoading;
};




// Builds cached glyph textures from shape info.    

extern void GFx_GenerateFontBitmaps(FontPackParams *params,
                                    const Array<FontResource*>& fonts,
                                    ImageCreator *pimageCreator,
                                    Log* plog,
                                    ResourceId* pidGenerator,
                                    MemoryHeap* fontHeap,
                                    bool threadedLoading);


}} // namespace Scaleform { namespace GFx {
#endif // SF_NO_FONT_GLYPH_PACKER
#endif // INC_SF_GFX_FONTLIBIMPL_H
