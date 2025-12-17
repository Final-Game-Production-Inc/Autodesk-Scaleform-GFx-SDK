/**************************************************************************

PublicHeader:   FontProvider_PS3
Filename    :   GFx_FontProviderPS3.h
Content     :   PS3 Cell Font provider
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

#ifndef INC_SF_GFX_FontProviderPS3_H
#define INC_SF_GFX_FontProviderPS3_H

#include "Render/FontProvider/Render_FontProviderPS3.h"
#include "GFx/GFx_Loader.h"

namespace Scaleform { namespace GFx {

typedef Render::ExternalFontPS3 ExternalFontPS3;

class FontProviderPS3 : public FontProvider
{
    typedef Render::Font Font;
    Render::FontProviderPS3* getFontProvider();

public:
    FontProviderPS3(unsigned fontCacheSize=1024*1024, unsigned numUserFontEntries=8, unsigned firstId=0, MemoryHeap* heap=0);
    virtual ~FontProviderPS3();

    void MapSystemFont(const char* fontName, unsigned fontFlags, 
                       uint32_t fontType, uint32_t fontMap,
                       Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                       Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                       unsigned maxVectorHintedSize=24,
                       unsigned maxRasterHintedSize=24);

    void MapFontToFile(const char* fontName, unsigned fontFlags, 
                       const char* fileName, uint32_t subNum=0,
                       Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                       Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                       unsigned maxVectorHintedSize=24,
                       unsigned maxRasterHintedSize=24);

    void MapFontToMemory(const char* fontName, unsigned fontFlags, 
                         const char* fontData, unsigned dataSize, 
                         uint32_t subNum=0,
                         Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                         Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                         unsigned maxVectorHintedSize=24,
                         unsigned maxRasterHintedSize=24);

    virtual Font*       CreateFont(const char* name, unsigned fontFlags);
    virtual void        LoadFontNames(StringHash<String>& fontnames);
};

}} // namespace Scaleform { namespace GFx {

#endif
