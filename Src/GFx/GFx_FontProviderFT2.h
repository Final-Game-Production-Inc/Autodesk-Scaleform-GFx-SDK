/**************************************************************************

PublicHeader:   FontProvider_FT2
Filename    :   GFx_FontProviderFT2.h
Content     :   FreeType2 Font provider
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

#ifndef INC_SF_GFX_FontProviderFT2_H
#define INC_SF_GFX_FontProviderFT2_H

#include "Render/FontProvider/Render_FontProviderFT2.h"
#include "GFx/GFx_Loader.h"

namespace Scaleform { namespace GFx {

typedef Render::ExternalFontFT2 ExternalFontFT2;

class FontProviderFT2 : public FontProvider
{
   typedef Render::Font Font;
    Render::FontProviderFT2* getFontProvider();

public:
    FontProviderFT2(FT_Library lib=0, MemoryHeap* heap=0);

    FT_Library GetFT_Library();

    void MapFontToFile(const char* fontName, unsigned fontFlags, 
                       const char* fileName, unsigned faceIndex=0, 
                       Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                       Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                       unsigned maxVectorHintedSize=24,
                       unsigned maxRasterHintedSize=24);

    void MapFontToMemory(const char* fontName, unsigned fontFlags, 
                         const char* fontData, unsigned dataSize, 
                         unsigned faceIndex=0, 
                         Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                         Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                         unsigned maxVectorHintedSize=24,
                         unsigned maxRasterHintedSize=24);

    virtual Font*    CreateFont(const char* name, unsigned fontFlags);    
    virtual void     LoadFontNames(StringHash<String>& fontnames);
};

}} // namespace Scaleform { namespace GFx {

#endif
