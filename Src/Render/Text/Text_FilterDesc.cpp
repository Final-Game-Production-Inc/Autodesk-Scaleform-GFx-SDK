/**************************************************************************

Filename    :   Text_FilterDesc.cpp
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Text_FilterDesc.h"
#include "Kernel/SF_Debug.h"
#include "Text_Core.h"
#include "Render/Render_GlyphParam.h"

namespace Scaleform { namespace Render { namespace Text {

void TextFilter::SetDefaultShadow()
{
    BlurX                 = 0;
    BlurY                 = 0;
    BlurStrength          = 1.0f;   

    ShadowParams.Mode     = 0;
    ShadowParams.BlurX    = 0;
    ShadowParams.BlurY    = 0;
    ShadowParams.Strength = 1.0f;   
    ShadowParams.Colors[0]= 0;    

    ShadowFlags    = GlyphParam::FineBlur;
    ShadowAlpha    = 255;
    ShadowAngle    = 3.1415926535897932384626433832795f / 4.0f;
    ShadowDistance = 4.0;
    UpdateShadowOffset();
}

// Loads a filter desc into a text filter object
//------------------------------------------------------------------------
void TextFilter::LoadFilterDesc(const Filter* filter)
{
    Render::FilterType filterType = filter->GetFilterType();

    if (filterType == Render::Filter_Blur)
    {
        const BlurFilter* f = (const BlurFilter*)filter;
        const BlurFilterParams &bfp = f->GetParams();
        BlurX        = bfp.BlurX;
        BlurY        = bfp.BlurY;
        BlurStrength = bfp.Strength;
        return;
    }
    else if ((filterType == Render::Filter_Shadow ||
              filterType == Render::Filter_Glow) && 
              (ShadowParams.Colors[0] == 0 || ShadowDistance == 0))
    {
        const BlurFilter* f = (const BlurFilter*)filter;
        const BlurFilterParams &bfp = f->GetParams();

        ShadowFlags = 0;
        if (bfp.Mode & BlurFilterParams::Mode_Knockout)
            ShadowFlags |= GlyphParam::KnockOut;
        if (bfp.Mode & BlurFilterParams::Mode_HideObject)
            ShadowFlags |= GlyphParam::HideObject;
        if (bfp.Passes >= 1)
            ShadowFlags |= GlyphParam::FineBlur;

        ShadowParams.Mode     = 0;
        ShadowParams.BlurX    = bfp.BlurX;
        ShadowParams.BlurY    = bfp.BlurY;
        ShadowParams.Strength = bfp.Strength;
        ShadowParams.Colors[0]= bfp.Colors[0];
        ShadowParams.Colors[1]= bfp.Colors[1];

        ShadowAlpha = ShadowParams.Colors[0].GetAlpha();
        ShadowAngle = f->GetAngle();
        ShadowDistance = f->GetDistance();
        UpdateShadowOffset();
        return;
    }    
}

}}} // Scaleform::Render::Text
