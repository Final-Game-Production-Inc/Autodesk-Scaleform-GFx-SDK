/**************************************************************************

Filename    :   GFx_FontProviderFT2.cpp
Content     :   FreeType2 font provider
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

#include "GFx/GFx_FontProviderFT2.h"
#include "Render/Render_Font.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace GFx {


FontProviderFT2::FontProviderFT2(FT_Library lib, MemoryHeap* heap)
: FontProvider(heap ? Ptr<Render::FontProviderFT2>(*SF_HEAP_NEW(heap) Render::FontProviderFT2(lib)).GetPtr() :
                      Ptr<Render::FontProviderFT2>(*SF_NEW            Render::FontProviderFT2(lib)).GetPtr())
{
}

Render::FontProviderFT2* FontProviderFT2::getFontProvider()
{
    return static_cast<Render::FontProviderFT2*>(pFontProvider.GetPtr());
}

void FontProviderFT2::MapFontToFile(const char* fontName, unsigned fontFlags, 
                                    const char* fileName, unsigned faceIndex, 
                                    Font::NativeHintingRange vectorHintingRange,
                                    Font::NativeHintingRange rasterHintingRange, 
                                    unsigned maxVectorHintedSize,
                                    unsigned maxRasterHintedSize)
{
    getFontProvider()->MapFontToFile(fontName, fontFlags, fileName, faceIndex, 
                                     vectorHintingRange, rasterHintingRange, 
                                     maxVectorHintedSize, maxRasterHintedSize);
}

void FontProviderFT2::MapFontToMemory(const char* fontName, unsigned fontFlags, 
                                      const char* fontData, unsigned dataSize, unsigned faceIndex, 
                                      Font::NativeHintingRange vectorHintingRange,
                                      Font::NativeHintingRange rasterHintingRange, 
                                      unsigned maxVectorHintedSize,
                                      unsigned maxRasterHintedSize)
{
    getFontProvider()->MapFontToMemory(fontName, fontFlags, fontData, dataSize, faceIndex, 
                                       vectorHintingRange, rasterHintingRange, 
                                       maxVectorHintedSize, maxRasterHintedSize);
}


Render::Font* FontProviderFT2::CreateFont(const char* name, unsigned fontFlags)
{
    return getFontProvider()->CreateFont(name, fontFlags);
}

void FontProviderFT2::LoadFontNames(StringHash<String>& fontnames)
{
    getFontProvider()->LoadFontNames(fontnames);
}

}} //namespace Scaleform { namespace GFx {
