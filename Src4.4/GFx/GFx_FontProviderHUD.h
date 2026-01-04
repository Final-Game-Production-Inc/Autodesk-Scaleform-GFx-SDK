/**************************************************************************

PublicHeader:   FontProvider_HUD
Filename    :   GFx_FontProviderHUD.h
Content     :   HUD Font provider, a single bitmap font
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

#ifndef INC_SF_GFX_FontProviderHUD_H
#define INC_SF_GFX_FontProviderHUD_H

#include "Render/FontProvider/Render_FontProviderHUD.h"
#include "GFx/GFx_Loader.h"

namespace Scaleform { namespace GFx {


typedef Render::ExternalFontHUD ExternalFontHUD;

class FontProviderHUD : public FontProvider
{
    typedef Render::Font Font;
    Render::FontProviderHUD* getFontProvider();

public:
    FontProviderHUD();
    FontProviderHUD(MemoryHeap* heap);

    virtual Font*    CreateFont(const char* name, unsigned fontFlags);    
    virtual void     LoadFontNames(StringHash<String>& fontnames);
};

}} // namespace Scaleform { namespace GFx {

#endif
