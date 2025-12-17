#ifndef GFX_EXPORT_VISITORS_H
#define GFX_EXPORT_VISITORS_H

#include "GFx/GFx_Player.h"
#include "GFxExport.h"

using namespace Scaleform;
using namespace GFx;

class GFxImageExtractorVisitor : public MovieDef::ResourceVisitor
{
public:
    GFxDataExporter& Exporter;
    String        Name;
    String        DestPath;

    GFxImageExtractorVisitor(GFxDataExporter& exporter, const String& name, const String& destPath) :
    Exporter(exporter), Name(name), DestPath(destPath) { }

    virtual void Visit(MovieDef* pmovieDef, Resource* presource, ResourceId rid, const char* pexportName);

    inline GFxImageExtractorVisitor& operator=(const GFxImageExtractorVisitor&) { return *this; } // dummy
};

struct GFxFontsVisitor : public MovieDef::ResourceVisitor
{
    GFxDataExporter* pExporter;
    ResourceBinding* pResBinding;

    GFxFontsVisitor(GFxDataExporter* pexporter, ResourceBinding* presbinding) : 
    pExporter(pexporter), pResBinding(presbinding) {}

    virtual void Visit(MovieDef*, Resource* presource, ResourceId rid, const char*);
};

struct GFxFontNameVisitor : public MovieDef::ResourceVisitor
{        
    Array<String>& Fonts;

    GFxFontNameVisitor(Array<String>& farr) : Fonts(farr) {}

    virtual void Visit(MovieDef*, Resource* presource, ResourceId rid, const char*);

    GFxFontNameVisitor& operator=(const GFxFontNameVisitor&) { return *this; } // warning suppression
};

struct GFxFontTexturesVisitor : public TextureGlyphData::TexturesVisitor
{
    FontTexturesHashType*               pTextures;
    int                                 FontId;
    Ptr<FontResource>                   pFont;

    inline GFxFontTexturesVisitor(FontTexturesHashType& textures, int fontId, FontResource* pfont) : 
        pTextures(&textures), FontId(fontId), pFont(pfont) {}

    void Visit(ResourceId textureResId, ImageResource* pimageRes);
};

struct GFxTextureGlyphVisitor : public TextureGlyphData::TextureGlyphVisitor
{
    ResourceBinding*             pResBinding;
    GFxFontTextureDescr&            TextureDescr;
    GFxFontTextureDescr::FontDescr& FntDescr;
    unsigned                            NumGlyphsInFont;

    GFxTextureGlyphVisitor(GFxFontTextureDescr& textureDescr, 
                GFxFontTextureDescr::FontDescr& fntDescr, 
                ResourceBinding* presbinding) : 
            pResBinding(presbinding), TextureDescr(textureDescr), FntDescr(fntDescr), NumGlyphsInFont(0) {}

            void Visit(unsigned index, GFx::TextureGlyph* ptextureGlyph);
    GFxTextureGlyphVisitor& operator=(const GFxTextureGlyphVisitor&) { return *this; } // warning suppression
};

struct GFxTextFieldVisitor : public MovieDef::ResourceVisitor
{        
    Array<String>& Text;

    GFxTextFieldVisitor(Array<String>& tarr) : Text(tarr) {}

    virtual void Visit(MovieDef*, Resource* presource, ResourceId rid, const char*);
    void AddUniqueTextToArray(const String& txt);
    GFxTextFieldVisitor& operator=(const GFxTextFieldVisitor&) { return *this; } // warning suppression
};

struct GFxEditTextFieldVisitor : public MovieDef::ResourceVisitor
{        
    Array<String>& Text;
    bool DynOnly;

    GFxEditTextFieldVisitor(Array<String>& tarr, bool dynOnly = false) : Text(tarr), DynOnly(dynOnly) {}

    virtual void Visit(MovieDef*, Resource* presource, ResourceId, const char*);

    void AddUniqueTextToArray(const String& txt);

    GFxEditTextFieldVisitor& operator=(const GFxEditTextFieldVisitor&) { return *this; } // warning suppression
};

// calculate total number of images
struct GFxImageSoundCounterVisitor : public MovieDef::ResourceVisitor
{
    int ImageCount;
    int SoundCount;
    GFxImageSoundCounterVisitor() : ImageCount(0), SoundCount(0) {}

    virtual void Visit(MovieDef*, Resource* presource, ResourceId, const char*);
};

// collect gradients' textures
struct GFxGradientsVisitor : public MovieDef::ResourceVisitor
{
    GFxDataExporter* pExporter;

    GFxGradientsVisitor(GFxDataExporter* pexporter) : pExporter(pexporter) {}

    virtual void Visit(MovieDef*, Resource* presource, ResourceId rid, const char*);
};


#ifdef GFX_ENABLE_SOUND

class GFxSoundExtractorVisitor : public MovieDef::ResourceVisitor
{
public:
    GFxDataExporter& Exporter;
    String        Name;
    String        DestPath;

    GFxSoundExtractorVisitor(GFxDataExporter& exporter, const String& name, const String& destPath) :
    Exporter(exporter), Name(name), DestPath(destPath) { }

    virtual void    Visit(MovieDef* pmovieDef, Resource* presource, ResourceId rid, const char* pexportName);

    inline GFxSoundExtractorVisitor& operator=(const GFxSoundExtractorVisitor&) { return *this; } // dummy
};

class GFxSpriteSoundExtractorVisitor : public MovieDef::ResourceVisitor
{
public:
    GFxDataExporter& Exporter;
    String        Name;
    String        DestPath;

    GFxSpriteSoundExtractorVisitor(GFxDataExporter& exporter, const String& name, const String& destPath) :
    Exporter(exporter), Name(name), DestPath(destPath) { }

    virtual void Visit(MovieDef* pmovieDef, Resource* presource, ResourceId rid, const char* pexportName);

    inline GFxSpriteSoundExtractorVisitor& operator=(const GFxSpriteSoundExtractorVisitor&) { return *this; } // dummy
};



#endif



#endif
