/**************************************************************************

PublicHeader:   Render
Filename    :   Text_FontManager.h
Content     :   Abstract font manager interface for text engine
Created     :   Oct 5, 2010
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef RENDER_TEXT_FONTMGR_H
#define RENDER_TEXT_FONTMGR_H

#include "Kernel/SF_Types.h"
#include "Render/Render_Stats.h"
#include "Render/Render_Font.h"

namespace Scaleform { 
namespace Render {
namespace Text {

class FontManagerBase;

class FontHandle : public RefCountBase<FontHandle, StatRender_Text_Mem>
{
protected:
    friend class FontManagerBase;

    // Font manager which we come from. Can be null for static text fields.
    FontManagerBase*    pFontManager;

    // A font device flag applied to a non-device font, as can be done
    // if the font is taken from FontLib due to its precedence.
    unsigned            OverridenFontFlags;   

    // Search FontName - could be an export name instead of the
    // name of the font.
    StringLH            FontName;

    float               FontScaleFactor;

    // Font we are holding.
    Ptr<Font>           pFont;
public:

    // A strong pointer to MovieDefImpl from which the font was created.
    enum FontFlags
    {
        // General font style flags.
        FF_Italic               = Font::FF_Italic,
        FF_Bold                 = Font::FF_Bold,
        FF_BoldItalic           = Font::FF_BoldItalic,
        FF_Style_Mask           = Font::FF_Style_Mask,

        FF_NoAutoFit            = 0x20
    };

    FontHandle(FontManagerBase *pmanager, Font* pfont,
               const char* pfontName = 0, unsigned overridenFontFlags = 0)
        : pFontManager(pmanager), OverridenFontFlags(overridenFontFlags),
          FontScaleFactor(1.0f), pFont(pfont)
    {
        // If font name doesn't match, store it locally.   
        if (pfontName && (String::CompareNoCase(pfont->GetName(), pfontName) != 0))
            FontName = pfontName;
    }
    FontHandle(const FontHandle& f) : pFontManager(f.pFontManager),
        OverridenFontFlags(f.OverridenFontFlags), FontName(f.FontName),
        FontScaleFactor(f.FontScaleFactor), pFont(f.pFont)
    {
    }
    virtual ~FontHandle() {}
    
    Font*               GetFont() const { return pFont; }
    const char*         GetFontName() const
    {
        return FontName.IsEmpty() ? GetFont()->GetName() : FontName.ToCStr();
    }
    unsigned            GetFontFlags() const
    {
        return GetFont()->GetFontFlags() | OverridenFontFlags;
    }
    bool                IsFauxBold() const       { return (OverridenFontFlags & FF_Bold) != 0; }
    bool                IsFauxItalic() const     { return (OverridenFontFlags & FF_Italic) != 0; }
    bool                IsFauxBoldItalic() const { return (OverridenFontFlags & FF_BoldItalic) == FF_BoldItalic; }
    unsigned            GetFauxFontStyle() const { return (OverridenFontFlags & FF_Style_Mask); }
    unsigned            GetFontStyle() const     
    { 
        return ((GetFont()->GetFontFlags() | GetFauxFontStyle()) & FF_Style_Mask);
    }
    bool                IsAutoFitDisabled() const { return (OverridenFontFlags & FF_NoAutoFit) != 0; }

    float               GetFontScaleFactor() const { return FontScaleFactor; }

    bool                operator==(const FontHandle& f) const
    {
        return pFontManager == f.pFontManager && pFont == f.pFont && 
               OverridenFontFlags == f.OverridenFontFlags && FontName == f.FontName &&
               FontScaleFactor == f.FontScaleFactor;
    }
};

// Abstract font manager interface for text engine
class FontManagerBase : public RefCountBase<FontManagerBase, StatRender_Text_Mem>
{
    friend class FontHandle;    
public:

    // Structure for retrieving a font's full search path. It is 
    // used in cases if a font is not found for generating more
    // informative error message
    struct FontSearchPathInfo
    {
        FontSearchPathInfo(int indent = 0) : Indent(indent) {}

        int             Indent;
        StringBuffer    Info;
    };

    virtual ~FontManagerBase() {}


    // Returns font by name and style. If a non-NULL searchInfo passed to the method
    // it assumes that it was called only for a diagnostic purpose and in this case font
    // will not be searched in the internal cache and a created font handle will not be cached.
    virtual FontHandle*         CreateFontHandle(const char* pfontName, unsigned matchFontFlags, 
        bool allowListOfFonts = true, FontSearchPathInfo* searchInfo = NULL) =0; 

    // Returns any font with the font name 'pfontName' or the first one
    // in the hash.
    //FontHandle*          CreateAnyFontHandle(const char* pfontName, FontSearchPathInfo* searchInfo = NULL);

    virtual FontHandle*         GetEmptyFont() =0;

    // Clean internal cache. This method is called from Advance method of 
    // MovieRoot then it detects that FontLib, FontMap, FontProvider or Translator 
    // is changed
    virtual void                CleanCache() =0;


    FontHandle*                 CreateFontHandle(const char* pfontName, bool bold, bool italic, bool device, 
        bool allowListOfFonts = true, FontSearchPathInfo* searchInfo = NULL)
    { 
        unsigned matchFontFlags = ((bold) ? Font::FF_Bold : 0) |
            ((italic) ? Font::FF_Italic : 0) |
            ((device) ? Font::FF_DeviceFont : 0);
        return CreateFontHandle(pfontName, matchFontFlags, allowListOfFonts, searchInfo);
    }

};

}}}

#endif //RENDER_TEXT_FONTMGR_H

