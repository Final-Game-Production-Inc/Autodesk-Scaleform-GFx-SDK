/**************************************************************************

Filename    :   GFx_FontProviderWin32.cpp
Content     :   Win32 API Font provider (GetGlyphOutline)
Created     :   6/21/2007
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

----------------------------------------------------------------------
----The code of these classes was taken from the Anti-Grain Geometry
Project and modified for the use by Scaleform. 
Permission to use without restrictions is hereby granted to 
Scaleform Corporation by the author of Anti-Grain Geometry Project.
See http://antigtain.com for details.
**************************************************************************/

#include "GFx/GFx_FontProviderWin32.h"
#include "Render/Render_Font.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace GFx {


FontProviderWin32::FontProviderWin32(HDC dc) 
: FontProvider(Ptr<Render::FontProviderWinAPI>(*SF_NEW Render::FontProviderWinAPI(dc)).GetPtr())
{
}

FontProviderWin32::FontProviderWin32(HDC dc, MemoryHeap* heap)
: FontProvider(Ptr<Render::FontProviderWinAPI>(*SF_HEAP_NEW(heap) Render::FontProviderWinAPI(dc)).GetPtr())
{
}

Render::FontProviderWinAPI* FontProviderWin32::getFontProvider()
{
    return static_cast<Render::FontProviderWinAPI*>(pFontProvider.GetPtr());
}

void FontProviderWin32::SetHintingAllFonts(Font::NativeHintingRange vectorRange, 
                                           Font::NativeHintingRange rasterRange, 
                                           unsigned maxVectorHintedSize,
                                           unsigned maxRasterHintedSize)
{
    getFontProvider()->SetHintingAllFonts(vectorRange, rasterRange, maxVectorHintedSize, maxRasterHintedSize);
}

void FontProviderWin32::SetHinting(const char* name, 
                                   Font::NativeHintingRange vectorRange, 
                                   Font::NativeHintingRange rasterRange, 
                                   unsigned maxVectorHintedSize,
                                   unsigned maxRasterHintedSize)
{
    getFontProvider()->SetHinting(name, vectorRange, rasterRange, maxVectorHintedSize, maxRasterHintedSize);  
}

Render::Font* FontProviderWin32::CreateFont(const char* name, unsigned fontFlags)
{
    return getFontProvider()->CreateFont(name, fontFlags);
}

void FontProviderWin32::LoadFontNames(StringHash<String>& fontnames)
{
    getFontProvider()->LoadFontNames(fontnames);
}

}} //namespace Scaleform { namespace GFx {
