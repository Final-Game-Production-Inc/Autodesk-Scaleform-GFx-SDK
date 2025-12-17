/**************************************************************************

Filename    :   GFx_FontLib.cpp
Content     :   Implementation of font sharing and lookup through
                a list of registed MovieDef's.
Created     :   July 9, 2007
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_FontLib.h"
#include "GFx/GFx_FontResource.h"

#include "GFx/GFx_MovieDef.h"
#include "GFx/GFx_LoadProcess.h"

namespace Scaleform { namespace GFx {

// ***** FontLib class

// FontLib stores smart pointers to MovieDataDef instead of 
// MovieDefImpl. This is important to avoid circular ref-counting
// cycles between the states, movie def and font lib. The actual
// DefImpl is instead looked up through CreateMovie, which in most cases
// will just return a new pointer from ResourceLib.
// An additional benefit we get from such late binding is the use of
// correct states for the imported font movies; these states are taken
// from the context that referenced the font. These states affect
// texture packing, image creation and other aspects of loading.


// FontLibImpl is used to hide the implementation details of FontLib.
class FontLibImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:
   
    // A list of MovieDataDef pointers to movies
    // that serve as a source for fonts
    Array<Ptr<MovieDataDef> >  FontMovies;

    String          FileToSubstitute;
};


//------------------------------------------------------------------------
FontLib::FontLib()
   : State(State::State_FontLib),
     pImpl(new FontLibImpl)
{
    pImpl->FileToSubstitute = "gfxfontlib.swf"; // default value
}

FontLib::~FontLib()
{
    delete pImpl;
}

void     FontLib::AddFontsFrom(MovieDef* md, bool pin)
{
    MovieDefImpl *pmdi = (MovieDefImpl*)md;

    if (pImpl && pmdi)
    {        
        pImpl->FontMovies.PushBack(pmdi->GetDataDef());
        if (pin)
           md->PinResource();
    }
}

bool     FontLib::AreAllMoviesLoaded() const
{
    if (!pImpl)
        return true;
    for (UPInt i = 0; i<pImpl->FontMovies.GetSize(); i++)
    {
        MovieDataDef *pdataDef = pImpl->FontMovies[i];
        if (pdataDef->GetLoadState() < MovieDataDef::LS_LoadFinished)
            return false;
    }
    return true;
}

// *** FontResult - stores smart pointers to result with hidden implementation

void    FontLib::FontResult::SetResult(MovieDef* pmovieDef, FontResource* pfont)
{
    if (pmovieDef)
        pmovieDef->AddRef();
    if (pfont)
        pfont->AddRef();

    if (pMovieDef)
        pMovieDef->Release();
    if (pFontResource)
        pFontResource->Release();

    pMovieDef     = pmovieDef;
    pFontResource = pfont;        
}

FontLib::FontResult::~FontResult()
{
    if (pMovieDef)
        pMovieDef->Release();
    if (pFontResource)
        pFontResource->Release();
}



// Finds or creates a font of specified style by searching all of the
// registered MovieDef objects. The font resource is created to be compatible
// with puserDef's binding states and utilizing pstates for non-binding
// state values.
bool FontLib::FindFont(FontResult *presult,
                       const char* pfontname, unsigned fontFlags,
                       MovieDef* puserDef, StateBag *pstates, ResourceWeakLib* pweakLib)
{    
    // We keep device font bits for matching so that FontManager can correctly
    // try device fonts before non-device ones.
    fontFlags &= Font::FF_CreateFont_Mask| Font::FF_DeviceFont;

    // First, find a matching font in our MovieDefs.
    if (!pImpl)
        return 0;

    unsigned          i, fontBindIndex = 0;
    MovieDataDef *    pdataDef = 0;
    bool              fontFound = 0;
    Ptr<MovieDefImpl> pfontDefImpl;

    for (i = 0; (i<pImpl->FontMovies.GetSize()) && !fontFound; i++)
    {
        // TBD: We will need to do something about threading here, since it is legit
        // to call GetFontResource while loading still hasn't completed.
        pdataDef = pImpl->FontMovies[i];

        // Make sure that MovieDataDef has finished loading,
        // otherwise all fonts wouldn't be there yet.
        pdataDef->WaitForLoadFinish();

        // Font traversal IS thread safe due to the way linked list is updated.
        FontDataUseNode* pfont = pdataDef->GetFirstFont();

        for(; pfont != 0; pfont = pfont->pNext)
        {
            if (pfont->pFontData->MatchFont(pfontname, fontFlags))
            {
                fontBindIndex = pfont->BindIndex;
                fontFound     = 1;
                break;
            }
        }
    }

    if (!fontFound)
        return 0;

    if (puserDef)
    {
        MovieDefImpl* pudi = (MovieDefImpl*) puserDef;

        // Create loadStates with:
        //  1) Binding states from puserDef
        //  2) Dynamic states from psharedStates. These will probably come
        //     from GFxMovieImpl for the instance which is using this font.
        Ptr<LoadStates> pls = *new LoadStates(pudi->pLoaderImpl,
            pstates,
            pudi->pBindStates);    
        // Get movieDef. Most of the time this will just look up MovieDefImpl
        // in the library without performing the actual binding.
        pfontDefImpl = 
            *pudi->pLoaderImpl->CreateMovie_LoadState(pls, pdataDef, pudi->GetLoadFlags() | 
            Loader::LoadWaitCompletion);
    }
    else if (pweakLib)
    {
        // if userDef is NULL - try to create a MovieDefImpl from scratch.
        // This is a hacky solution and useful only for DrawText API with using FontLib.

        // Create loadStates with:
        //  1) New binding states with states from 'pstates'
        //  2) Resource lib from parameter
        Ptr<LoaderImpl> loaderImpl = *new LoaderImpl(pstates, pweakLib->GetResourceLib());
        Ptr<MovieDefBindStates> bs = *new MovieDefBindStates(pstates);
        Ptr<LoadStates> pls = *new LoadStates(loaderImpl, pstates, bs);    

        // Get movieDef. Most of the time this will just look up MovieDefImpl
        // in the library without performing the actual binding.
        pfontDefImpl = 
            *loaderImpl->CreateMovie_LoadState(pls, pdataDef, Loader::LoadWaitCompletion);
    }
    if (!pfontDefImpl)
        return 0;

    // TBD: Threading - Should we wait for binding to complete?
    ResourceBindData rbd;
    pfontDefImpl->GetResourceBinding().GetResourceData(&rbd, fontBindIndex);
    if (rbd.pResource)
    {
        SF_ASSERT(rbd.pResource->GetResourceType() == Resource::RT_Font);        
        presult->SetResult(pfontDefImpl, (FontResource*) rbd.pResource.GetPtr());
        return 1;        
    }           
    return 0;
}

// Loads font names into destination string array
void FontLib::LoadFontNames(StringHash<String>& fontnames)
{
    if (!pImpl)
        return;

    for (unsigned i = 0; (i<pImpl->FontMovies.GetSize()); i++)
    {
        // TBD: We will need to do something about threading here, since it is legit
        // to call GetFontResource while loading still hasn't completed.
        MovieDataDef* pdataDef = pImpl->FontMovies[i];

        // Make sure that MovieDataDef has finished loading,
        // otherwise all fonts wouldn't be there yet.
        pdataDef->WaitForLoadFinish();

        // Font traversal IS thread safe due to the way linked list is updated.
        FontDataUseNode* pfont = pdataDef->GetFirstFont();

        for(; pfont != 0; pfont = pfont->pNext)
        {
            String fontname(pfont->pFontData->GetName());
            fontnames.Set(fontname, fontname);
        }
    }
}

void FontLib::LoadFonts(HashSet<Ptr<Render::Font> >& fonts)
{
    if (!pImpl)
        return;

    for (unsigned i = 0; (i<pImpl->FontMovies.GetSize()); i++)
    {
        // TBD: We will need to do something about threading here, since it is legit
        // to call GetFontResource while loading still hasn't completed.
        MovieDataDef* pdataDef = pImpl->FontMovies[i];

        // Make sure that MovieDataDef has finished loading,
        // otherwise all fonts wouldn't be there yet.
        pdataDef->WaitForLoadFinish();

        // Font traversal IS thread safe due to the way linked list is updated.
        FontDataUseNode* pfont = pdataDef->GetFirstFont();

        for(; pfont != 0; pfont = pfont->pNext)
        {
            fonts.Set(pfont->pFontData);
        }
    }
}

//void FontLib::SetSubstitute(const char* filename)
//{
//    pImpl->FileToSubstitute = filename;
//}

//const String& FontLib::GetSubstitute() const
//{
//    return pImpl->FileToSubstitute;
//}

//////////////////////////////////////////////////////////////////////////
void StateBag::SetDefaultFontLibName(const char* defaultFontLib)
{
    Ptr<DefaultFontLibNameState> dfn = *new DefaultFontLibNameState();
    if (dfn)
    {
        dfn->SetName(defaultFontLib);
    }
    SetState(State::State_DefaultFontLibName, dfn);
}

const char* StateBag::GetDefaultFontLibName() const
{
    Ptr<DefaultFontLibNameState> dfn = *((DefaultFontLibNameState*) GetStateAddRef(State::State_DefaultFontLibName));
    if (dfn)
        return dfn->GetName().ToCStr();
    else
        return 0;
}


}} //namespace Scaleform { namespace GFx {



