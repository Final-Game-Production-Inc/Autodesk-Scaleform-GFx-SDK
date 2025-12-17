/**************************************************************************

Filename    :   GFx_FontMap.cpp
Content     :   Implementation of font name mapping.
Created     :   July 18, 2007
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_Loader.h"
#include "GFx/GFx_StringHash.h"
#include "Kernel/SF_Debug.h"

namespace Scaleform { namespace GFx {

// ***** FontMap class

// FontMapImpl contains the font mapping hash table.
class FontMapImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:    

    StringHashLH<FontMap::MapEntry>    FontMapValue;       
};


FontMap::FontMap()
   : State(State::State_FontMap),
     pImpl(new FontMapImpl)
{
}

FontMap::~FontMap()
{
    delete pImpl;
}

bool    FontMap::MapFont(const char* pfontName, const char* pnewFontName, MapFontFlags mff, float scaleFactor)
{
    if (!pImpl || !pfontName)
        return 0;

    String fontName(pfontName);    
    if (pImpl->FontMapValue.GetCaseInsensitive(fontName))
    {
        SF_DEBUG_WARNING1(1, "FontMap::MapFont failed - font name '%s' already mapped",
                           fontName.ToCStr());
        return 0;
    }
    
    MapEntry  me(pnewFontName, mff, scaleFactor);
    pImpl->FontMapValue.Set(fontName, me);
    return 1;
}

bool    FontMap::MapFont(const wchar_t* pfontName, const wchar_t* pnewFontName, MapFontFlags mff, float scaleFactor)
{
    if (!pImpl || !pfontName)
        return 0;
  
    String fontName(pfontName);
    if (pImpl->FontMapValue.GetCaseInsensitive(fontName))
    {
        SF_DEBUG_WARNING1(1, "FontMap::MapFont failed - font name '%s' already mapped",
                           fontName.ToCStr());
        return 0;
    }

    MapEntry  me = MapEntry(String(pnewFontName), mff, scaleFactor);
    pImpl->FontMapValue.Set(fontName, me);
    return 1;
}

// Obtains a font mapping, returning a const string (UTF8).
bool        FontMap::GetFontMapping(MapEntry* pentry, const char *pfontName)
{
    if (!pImpl)
        return 0;

    const MapEntry* pfound = pImpl->FontMapValue.GetCaseInsensitive(pfontName);   
    if (pfound)
    {
        *pentry = *pfound;
        return 1;
    }
    return 0;
}
}} //namespace Scaleform { namespace GFx {
