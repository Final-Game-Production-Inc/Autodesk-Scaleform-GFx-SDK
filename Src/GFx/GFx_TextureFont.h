/**********************************************************************

Filename    :   GFx_TextureFont.h
Content     :   Application-created texture fonts.
Created     :   Aug 2011
Authors     :   Andrew Reisse, Vlad Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFxTextureFont_H
#define INC_GFxTextureFont_H

#include "Render/Render_Font.h"
#include "GFx/GFx_FontResource.h"
#include "GFx/GFx_ImageResource.h"
#include "GFx/GFx_Loader.h"


namespace Scaleform { namespace GFx {

//------------------------------------------------------------------------

class TextureFont : public Render::Font
{
public:
    struct AdvanceEntry
    {
        float       Advance;
        SInt16      Left;
        SInt16      Top;
        UInt16      Width;
        UInt16      Height;
    };

    TextureFont(const char* name, unsigned fontFlags, unsigned numGlyphs);
    ~TextureFont();

    void            SetTextureParams(FontPackParams::TextureConfig params, unsigned flags, unsigned padding)
    {
        Flags |= flags;
        Padding = padding;
        pTextureGlyphData->SetTextureConfig(params);
    }
    void            AddTextureGlyph(unsigned glyphIndex, ImageResource* pimage,
                                    Render::RectF& UvBounds, Render::PointF& UvOrigin, AdvanceEntry advance);
    inline void     SetCharMap(UInt16 from, UInt16 to)
    {
        CodeTable.Add(from, to);
    }
    void            SetFontMetrics(float leading, float ascent, float descent)
    {
        Leading = leading;
        Ascent  = ascent;
        Descent = descent;
    }

    // *** Render::Font implementation

    virtual const TextureGlyph* GetTextureGlyph(unsigned glyphIndex) const
    {
        return (pTextureGlyphData) ? &pTextureGlyphData->GetTextureGlyph(glyphIndex) : NULL;
    }
    virtual float GetTextureGlyphHeight() const
    { 
        return (pTextureGlyphData) ? float(pTextureGlyphData->GetNominalSize() * 1024.0 / 1536.0) : 0;
    }
    virtual void* GetTextureGlyphData() const { return pTextureGlyphData; }

    virtual float           GetAdvance(unsigned glyphIndex) const;
    virtual int             GetGlyphIndex(UInt16 code);
    virtual int             GetCharValue(unsigned glyphIndex) const;
    virtual float           GetGlyphWidth(unsigned glyphIndex) const;
    virtual float           GetGlyphHeight(unsigned glyphIndex) const;
    virtual Render::RectF&  GetGlyphBounds(unsigned glyphIndex, Render::RectF* prect) const;

    virtual float           GetKerningAdjustment(unsigned, unsigned) const { return 0; }
    virtual const           ShapeDataInterface* GetPermanentGlyphShape(unsigned) const { return NULL; }

    virtual const char*     GetName() const { return Name; }

    // *** Other APIs

    inline bool             HasVectorOrRasterGlyphs() const { return true; }    

private:
    char*                          Name;
    Ptr<TextureGlyphData>          pTextureGlyphData;
    HashIdentityLH<UInt16, UInt16> CodeTable;
    ArrayLH<AdvanceEntry>          AdvanceTable;
    unsigned                       Padding;
};

}} // Scaleform::GFx

#endif
