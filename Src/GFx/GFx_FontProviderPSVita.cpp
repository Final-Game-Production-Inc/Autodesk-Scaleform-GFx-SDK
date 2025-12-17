/**************************************************************************

Filename    :   GFx_FontProviderPSVita.cpp
Content     :
Created     :   2011
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

#include "GFx/GFx_FontProviderPSVita.h"
#include "Render/Render_Font.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace GFx {


FontProviderPSVita::FontProviderPSVita(unsigned numUserFontEntries, int firstId)
: FontProvider(Ptr<Render::FontProviderPSVita>(*SF_NEW Render::FontProviderPSVita(numUserFontEntries, firstId)).GetPtr())
{
}

FontProviderPSVita::FontProviderPSVita(MemoryHeap* heap, unsigned numUserFontEntries, int firstId)
: FontProvider(Ptr<Render::FontProviderPSVita>(*SF_HEAP_NEW(heap) Render::FontProviderPSVita(numUserFontEntries, firstId)).GetPtr())
{
}

Render::FontProviderPSVita* FontProviderPSVita::getFontProvider()
{
    return static_cast<Render::FontProviderPSVita*>(pFontProvider.GetPtr());
}

void FontProviderPSVita::MapSystemFont(const char* fontName, unsigned fontFlags, unsigned fontType, 
                                    Render::PSVita_FontStreamType fontStream,
                                    float metricsScale, float advanceScale)
{
    getFontProvider()->MapSystemFont(fontName, fontFlags, fontType, fontStream,
                                     metricsScale, advanceScale);
}

void FontProviderPSVita::MapFontToFile(const char* fontName, unsigned fontFlags, const char* fileName, 
                                    Render::PSVita_FontStreamType fontStream,
                                    float metricsScale, float advanceScale)
{
    getFontProvider()->MapFontToFile(fontName, fontFlags, fileName, fontStream,
                                     metricsScale, advanceScale);
}

void FontProviderPSVita::MapFontToMemory(const char* fontName, unsigned fontFlags, 
                                      const char* fontData, unsigned dataSize, 
                                      float metricsScale, float advanceScale)
{
    getFontProvider()->MapFontToMemory(fontName, fontFlags, fontData, dataSize, 
                                       metricsScale, advanceScale);
}

void FontProviderPSVita::LoadFontNames(StringHash<String>& fontnames)
{
    getFontProvider()->LoadFontNames(fontnames);
}

}} //namespace Scaleform { namespace GFx {
