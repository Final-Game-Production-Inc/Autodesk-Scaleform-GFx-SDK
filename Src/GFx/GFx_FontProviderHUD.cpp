/**************************************************************************

Filename    :   GFx_FontProviderHUD.cpp
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

#include "GFx/GFx_FontProviderHUD.h"
#include "Render/Render_Font.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace GFx {


FontProviderHUD::FontProviderHUD() 
: FontProvider(Ptr<Render::FontProviderHUD>(*SF_NEW Render::FontProviderHUD()).GetPtr())
{
}

FontProviderHUD::FontProviderHUD(MemoryHeap* heap) 
: FontProvider(Ptr<Render::FontProviderHUD>(*SF_HEAP_NEW(heap) Render::FontProviderHUD()).GetPtr())
{
}

Render::FontProviderHUD* FontProviderHUD::getFontProvider()
{
    return static_cast<Render::FontProviderHUD*>(pFontProvider.GetPtr());
}

Render::Font* FontProviderHUD::CreateFont(const char* name, unsigned fontFlags)
{
    return getFontProvider()->CreateFont(name, fontFlags);
}

void FontProviderHUD::LoadFontNames(StringHash<String>& fontnames)
{
    getFontProvider()->LoadFontNames(fontnames);
}

}} //namespace Scaleform { namespace GFx {
