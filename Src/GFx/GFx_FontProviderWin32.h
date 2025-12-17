/**************************************************************************

PublicHeader:   FontProvider_Win32
Filename    :   GFx_FontProviderWin32.h
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

#ifndef INC_SF_GFX_FontProviderWin32_H
#define INC_SF_GFX_FontProviderWin32_H

#include "Render/FontProvider/Render_FontProviderWinAPI.h"
#include "GFx/GFx_Loader.h"

namespace Scaleform { namespace GFx {


typedef Render::ExternalFontWinAPI ExternalFontWin32;

class FontProviderWin32 : public FontProvider
{
    typedef Render::Font Font;
    Render::FontProviderWinAPI* getFontProvider();

public:
    FontProviderWin32(HDC dc);
    FontProviderWin32(HDC dc, MemoryHeap* heap);


    void SetHintingAllFonts(Font::NativeHintingRange vectorRange, 
                            Font::NativeHintingRange rasterRange, 
                            unsigned maxVectorHintedSize=24,
                            unsigned maxRasterHintedSize=24);

    void SetHinting(const char* name, 
                    Font::NativeHintingRange vectorRange, 
                    Font::NativeHintingRange rasterRange, 
                    unsigned maxVectorHintedSize=24,
                    unsigned maxRasterHintedSize=24);

    virtual Font*    CreateFont(const char* name, unsigned fontFlags);    

    virtual void     LoadFontNames(StringHash<String>& fontnames);
};

}} // namespace Scaleform { namespace GFx {

#endif
