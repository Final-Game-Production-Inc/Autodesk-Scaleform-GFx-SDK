/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_FontLib.h
Content     :   Implementation of font sharing and lookup through
                a list of registed MovieDef's.
Created     :   July 9, 2007
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_FontLib_H
#define INC_SF_GFX_FontLib_H

#include "GFx/GFx_Loader.h"

namespace Scaleform { namespace GFx {

// ***** FontLib

// An user-managed state used to register MovieDefs so that they can 
// be searched for fonts when creating other movies. Fonts in registered
// files form a library, which is searched before GFxFontProvider.
// Unlike GFxFontProvider, FontLib does not require system font support
// and is thus portable to consoles.

class FontResource;
class MovieDef;

class FontLib : public State
{
    class FontLibImpl *pImpl;
public:
    FontLib();
    virtual ~FontLib();

    // Registers a MovieDef that will be used as a source for fonts.
    // By default, the the movie is pinned in the loader so that its
    // textures don't get unloaded.
    void                AddFontsFrom(MovieDef* md, bool pin = 1);

    // A structure filled in if LookupFont succeeds. Combines a font
    // resource with MovieDef which it came from. It is important
    // for users to keep a reference to both for as long as font is used.
    class FontResult
    {
        friend class FontLib;
        // These should be GPtrs, but they are not for compiler compatibility.
        MovieDef*       pMovieDef;
        FontResource*   pFontResource;
        void            SetResult(MovieDef* pmovieDef, FontResource* pfont);
    public:
        FontResult() { pMovieDef = 0; pFontResource = 0; }
        ~FontResult();                   

        MovieDef*     GetMovieDef() const { return pMovieDef; }
        FontResource* GetFontResource() const { return pFontResource; }
    };

    // Finds or creates a font of specified style by searching all of the
    // registered MovieDef objects. The font resource is created to be compatible
    // with puserDef's binding states and utilizing pstates for non-binding
    // state values.
    // fontFlags style as described by GFxFont::FontFlags
    virtual bool        FindFont(FontResult *presult,
                                 const char* name, unsigned fontFlags,
                                 MovieDef* puserDef, StateBag *pstates, 
                                 ResourceWeakLib* pweakLib = NULL);

    // Checks if all font movies are completely loaded
    bool                AreAllMoviesLoaded() const;

    // Loads all available font names into destination array
    void                LoadFontNames(StringHash<String>& fontnames);
    void                LoadFonts(HashSet<Ptr<Render::Font> >& fonts);

    // Sets a filename of SWF with fonts to be substituted by fontlib. 
    // This should be a name of SWF file that was used as a font import 
    // library during authoring of SWF content in Flash Studio. For example,
    // if 'fonts_en.swf' was used as a default source of imported fonts then use
    // this name. Default value is 'gfxfontlib.swf' for compatibility reasons.
    //void                SetSubstitute(const char* filename);
    //const String&       GetSubstitute() const;
};

// *** Default Font Lib Name state
class DefaultFontLibNameState : public State
{
    StringLH Name;
public:
    DefaultFontLibNameState():State(State::State_DefaultFontLibName) {}
    virtual ~DefaultFontLibNameState() {}

    void            SetName(const char* n)  { Name = n; }
    const StringLH& GetName() const         { return Name; }
};


// *** External class inlines dependent on us

inline void StateBag::SetFontLib(FontLib* pri)
{
    SetState(State::State_FontLib, pri);
}
inline Ptr<FontLib>  StateBag::GetFontLib() const
{
    return *(FontLib*) GetStateAddRef(State::State_FontLib);
}

}} // Scaleform::GFx

#endif
