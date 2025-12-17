/**************************************************************************

PublicHeader:   Render
Filename    :   Text_FilterDesc.h
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Text_FilterDesc_H
#define INC_SF_Render_Text_FilterDesc_H

#include <math.h>
#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Render/Render_Stats.h"
#include "Render/Render_Color.h"
#include "Render/Render_Filters.h"

namespace Scaleform { namespace Render { namespace Text {

//------------------------------------------------------------------------
struct TextFilter : public RefCountBaseNTS<TextFilter, StatRender_Text_Mem>
{
    float            BlurX, BlurY;
    float            BlurStrength;
    BlurFilterParams ShadowParams;
    unsigned         ShadowFlags; // Use GlyphParam flags.
    float            ShadowAngle, ShadowDistance;    
    UInt8            ShadowAlpha;

    TextFilter()
    {
        SetDefaultShadow();
    }

    void SetDefaultShadow();

    void UpdateShadowOffset()
    {
        ShadowParams.Offset = 
            BlurFilterParams::CalcOffsetByAngleDistance(ShadowAngle, ShadowDistance);
    }

    void LoadFilterDesc(const Filter* filter);

    bool operator==(const TextFilter& f)
    {
        return  BlurX         ==  f.BlurX &&
                BlurY         ==  f.BlurY &&
                BlurStrength  ==  f.BlurStrength &&
                ShadowParams.EqualsAll(f.ShadowParams) &&
                ShadowFlags   ==  f.ShadowFlags &&
                ShadowAlpha   ==  f.ShadowAlpha &&
                ShadowAngle   ==  f.ShadowAngle &&
                ShadowDistance==  f.ShadowDistance &&
                ShadowAngle   ==  f.ShadowAngle;
    }
};


}}} // Scaleform::Render::Text

#endif // INC_SF_Render_Text_FilterDesc_H
