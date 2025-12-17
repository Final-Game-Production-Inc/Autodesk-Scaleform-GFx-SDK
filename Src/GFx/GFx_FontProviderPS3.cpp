/**************************************************************************

Filename    :   GFx_FontProviderPS3.cpp
Content     :   PS3 Cell font provider
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

#include "GFx/GFx_FontProviderPS3.h"
#include "Render/Render_Font.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace GFx {

FontProviderPS3::FontProviderPS3(unsigned fontCacheSize, unsigned numUserFontEntries, unsigned firstId, MemoryHeap* heap)
: FontProvider(heap ? Ptr<Render::FontProviderPS3>(*SF_HEAP_NEW(heap) Render::FontProviderPS3(fontCacheSize, numUserFontEntries, firstId)).GetPtr() :
                      Ptr<Render::FontProviderPS3>(*SF_NEW            Render::FontProviderPS3(fontCacheSize, numUserFontEntries, firstId)).GetPtr())
{
}

FontProviderPS3::~FontProviderPS3()
{
}

Render::FontProviderPS3* FontProviderPS3::getFontProvider()
{
    return static_cast<Render::FontProviderPS3*>(pFontProvider.GetPtr());
}

void FontProviderPS3::MapSystemFont(const char* fontName, unsigned fontFlags, 
                                    uint32_t fontType, uint32_t fontMap,
                                    Font::NativeHintingRange vectorHintingRange,
                                    Font::NativeHintingRange rasterHintingRange, 
                                    unsigned maxVectorHintedSize,
                                    unsigned maxRasterHintedSize)
{
    getFontProvider()->MapSystemFont(fontName, fontFlags, fontType, fontMap,
                                     vectorHintingRange, rasterHintingRange, 
                                     maxVectorHintedSize, maxRasterHintedSize);
}

void FontProviderPS3::MapFontToFile(const char* fontName, unsigned fontFlags, 
                                    const char* fileName, uint32_t subNum,
                                    Font::NativeHintingRange vectorHintingRange,
                                    Font::NativeHintingRange rasterHintingRange, 
                                    unsigned maxVectorHintedSize,
                                    unsigned maxRasterHintedSize)
{
    getFontProvider()->MapFontToFile(fontName, fontFlags, fileName, subNum,
                                     vectorHintingRange, rasterHintingRange, 
                                     maxVectorHintedSize, maxRasterHintedSize);
}

void FontProviderPS3::MapFontToMemory(const char* fontName, unsigned fontFlags, 
                                      const char* fontData, unsigned dataSize, 
                                      uint32_t subNum,
                                      Font::NativeHintingRange vectorHintingRange,
                                      Font::NativeHintingRange rasterHintingRange, 
                                      unsigned maxVectorHintedSize,
                                      unsigned maxRasterHintedSize)
{
    getFontProvider()->MapFontToMemory(fontName, fontFlags, fontData, dataSize, subNum,
                                       vectorHintingRange, rasterHintingRange, 
                                       maxVectorHintedSize, maxRasterHintedSize);
}

Render::Font* FontProviderPS3::CreateFont(const char* name, unsigned fontFlags)
{
    return getFontProvider()->CreateFont(name, fontFlags);
}

void FontProviderPS3::LoadFontNames(StringHash<String>& fontnames)
{
    getFontProvider()->LoadFontNames(fontnames);
}


}} //namespace Scaleform { namespace GFx {
