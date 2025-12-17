/**********************************************************************

Filename    :   GFx_TextureFont.cpp
Content     :   Application-created texture fonts.
Created     :   Aug 2011
Authors     :   Andrew Reisse, Vlad Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_TextureFont.h"

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_Debug.h"


namespace Scaleform { namespace GFx {

//------------------------------------------------------------------------

TextureFont::TextureFont(const char* name, unsigned fontFlags, unsigned numGlyphs) :
    Render::Font(fontFlags), Name(NULL)
{
    if (name)
    {
        Name = (char*)SF_ALLOC(SFstrlen(name) + 1, StatMD_Fonts_Mem);
        if (Name)
            SFstrcpy(Name, SFstrlen(name) + 1, name);
    }
    else
        Name = NULL;

    pTextureGlyphData = *SF_NEW TextureGlyphData(numGlyphs, true);
    Flags |= FF_HasLayout|FF_GlyphShapesStripped|FF_WideCodes;
}

TextureFont::~TextureFont()
{
    if (Name)
    {
        SF_FREE(Name);
        Name = NULL;
    }
}

void TextureFont::AddTextureGlyph(unsigned glyphIndex, ImageResource* pimage,
                                  Render::RectF& UvBounds, Render::PointF& UvOrigin,
                                  AdvanceEntry advance)
{
    TextureGlyph tg;
    tg.pImage = static_cast<Render::Image*>(pimage->GetImage());
    tg.UvBounds = UvBounds;
    tg.UvOrigin = UvOrigin;
    pTextureGlyphData->AddTextureGlyph(glyphIndex, tg);

    if (AdvanceTable.GetSize() <= glyphIndex)
    {
        AdvanceTable.Resize(glyphIndex + 1);
        if (AdvanceTable.GetSize() <= glyphIndex)
            return;
    }

    AdvanceTable[glyphIndex] = advance;
}


//------------------------------------------------------------------------

float TextureFont::GetAdvance(unsigned GlyphIndex) const
{
    if (GlyphIndex == (unsigned)-1)
        return GetNominalGlyphWidth();
 
    if (AdvanceTable.GetSize() == 0)
    {
        // No layout info for this TextureFont!
        static bool Logged = false;
        if (Logged == false)
        {
            Logged = true;
            SF_DEBUG_ERROR1(1, "Empty advance table in font %s", GetName());
        }
        // It is better to return default advance, make char substitution show up better
        return GetNominalGlyphWidth();
    }

    if (GlyphIndex < AdvanceTable.GetSize())
    {
        SF_ASSERT(GlyphIndex != (unsigned)-1);
        return AdvanceTable[GlyphIndex].Advance;
    }
    else
    {
        // Bad glyph index
        SF_ASSERT(0);
        return 0;
    }
}

int TextureFont::GetGlyphIndex(UInt16 code)
{
    UInt16 glyphIndex;
    if (CodeTable.Get(code, &glyphIndex))
    {
        return glyphIndex;
    }
    return -1;
}

int TextureFont::GetCharValue(unsigned glyphIndex) const
{
    for (HashIdentityLH<UInt16, UInt16>::ConstIterator iter = CodeTable.Begin();
        iter != CodeTable.End(); ++iter)
    {
        if (iter->Second == glyphIndex)
            return iter->First;
    }
    return -1;
}

float TextureFont::GetGlyphWidth(unsigned glyphIndex) const
{
    if (glyphIndex == (unsigned)-1)
        return GetNominalGlyphWidth();

    if (glyphIndex < AdvanceTable.GetSize())
    {
        SF_ASSERT(glyphIndex != (unsigned)-1);
        float w = float(AdvanceTable[glyphIndex].Width)/20.f;
        if (w != 0)
            return w;
    }
    return GetAdvance(glyphIndex); 
}

float TextureFont::GetGlyphHeight(unsigned glyphIndex) const
{
    if (glyphIndex == (unsigned)-1 || AdvanceTable.GetSize() == 0)    
        return GetNominalGlyphHeight();

    if (glyphIndex < AdvanceTable.GetSize())
    {
        SF_ASSERT(glyphIndex != (unsigned)-1);
        return float(AdvanceTable[glyphIndex].Height)/20.f;
    }
    else
    {
        // Bad glyph index
        SF_ASSERT(AdvanceTable.GetSize() == 0);
        return 0;
    }
}

Render::RectF& TextureFont::GetGlyphBounds(unsigned glyphIndex, Render::RectF* prect) const
{
    if (glyphIndex == (unsigned)-1)
    {
        prect->x1 = prect->y1 = 0;
        prect->SetWidth(GetGlyphWidth(glyphIndex));
        prect->SetHeight(GetGlyphHeight(glyphIndex));
    }
    else if (glyphIndex < AdvanceTable.GetSize())
    {
        SF_ASSERT(glyphIndex != (unsigned)-1);
        const AdvanceEntry& e = AdvanceTable[glyphIndex];
        float w = float(e.Width)/20.f;
        if (w == 0)
            w = e.Advance;
        float h = float(e.Height)/20.f;
        prect->x1 = float(e.Left)/20.f;
        prect->y1 = float(e.Top)/20.f;
        prect->SetWidth(w);
        prect->SetHeight(h);
    }
    else
    {
        // Bad glyph index
        SF_DEBUG_ERROR3((AdvanceTable.GetSize() != 0), 
            "Glyph index %d exceeds advance table (size = %d), font = %s", 
            glyphIndex, (int)AdvanceTable.GetSize(), GetName());

        prect->x1 = prect->y1 = 0;
        prect->SetWidth(0);
        prect->SetHeight(0);
    }
    return *prect;
}

}} // Scaleform::GFx
