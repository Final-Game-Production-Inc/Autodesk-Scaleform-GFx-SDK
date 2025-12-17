/**************************************************************************

Filename    :   Amp_Visitors.h
Content     :   Helper classes for memory fragmentation report
Created     :   March 2010
Authors     :   Maxim Shemenarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_AMP_MEMORY_IMAGE_H
#define INC_GFX_AMP_MEMORY_IMAGE_H

#include "GFxConfig.h"

#ifdef SF_AMP_SERVER

namespace Scaleform {
namespace GFx {
namespace AMP {

struct FontVisitor : public MovieDef::ResourceVisitor
{        
    Array<String> Fonts;

    virtual void Visit(MovieDef*, Resource* presource, ResourceId rid, const char*)
    {
        FontResource* pfontResource = static_cast<FontResource*>(presource);

        char buf[100];
        String font;
        font = pfontResource->GetName();
        if (pfontResource->GetFont()->IsBold())
        {
            font += " - Bold";
        }
        else if (pfontResource->GetFont()->IsItalic())
        {
            font += " - Italic";
        }
        SFsprintf(buf, sizeof(buf), ", %d glyphs", pfontResource->GetFont()->GetGlyphShapeCount()); 
        font += buf;
        if (!pfontResource->GetFont()->HasLayout())
        {
            font += ", static only";
        }
        font += " (";
        rid.GenerateIdString(buf);
        font += buf;
        font += ")";

        Fonts.PushBack(font);
    }
};

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif

#endif   // INC_GFX_AMP_MEMORY_IMAGE_H
