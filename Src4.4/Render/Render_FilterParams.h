/**************************************************************************

PublicHeader:   Render
Filename    :   Render_FilterParams.h
Content     :   
Created     :   
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_FilterParams_H
#define INC_SF_Render_FilterParams_H

//#include <math.h>
#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
//#include "Render/Render_Stats.h"
#include "Render/Render_Types2D.h"
#include "Render/Render_Color.h"
#include "Render/Render_Gradients.h"

namespace Scaleform { namespace Render {


//--------------------------------------------------------------------
// FilterType defines the type of filter that can be applied to
// as TreeNode filter states. On a high level, filters are separated 
// into "Blur" and "Other" filter families. All blur filters are
// described by a single low-level structure, BlurFilterParams.
// NOTE: These filter constant values do NOT map to Flash directly.

enum FilterType
{   
    // Blur filter family (8 low values reserved).
    Filter_Blur             = 0,
    Filter_Shadow           = 1,
    Filter_Glow             = 2,  // Shadow, offset 0.
    Filter_Bevel            = 3,
    Filter_GradientGlow     = 4,
    Filter_GradientBevel    = 5,
    Filter_Blur_End         = Filter_GradientBevel,

    // Non-blur filter implementations.
    Filter_ColorMatrix      = 8,
    Filter_Convolution      = 9,
    Filter_DisplacementMap  = 10,

    Filter_CacheAsBitmap,
};


//--------------------------------------------------------------------

// BlurFilterParams describes arguments for a Blur filter implementation.
// These arguments describe a blur filter state and are also passed into HAL.
// Filter parameter (BlurX/Y, Offset) coordinate system must match that of
// a shape and/or text-field that is applied to.

struct BlurFilterParams
{   
    enum ModeConstants
    {
        // Mask used to obtain filter type from Mode.
        Mode_FilterMask     = 0x07,
        // Flag bits that modify blur filter.
        Mode_Knockout       = 0x10,
        Mode_Inner          = 0x20,
        Mode_HideObject     = 0x40,
        Mode_Highlight      = 0x80,
        Mode_FlagsMask      = 0xF0
    };

    unsigned            Mode;           // Combination of ModeConstants and FilterType.
    unsigned            Passes;
    float               BlurX, BlurY;   // In Twips.
    PointF              Offset;
    float               Strength;       // [0..1]
    Color               Colors[2];      // Color[0] = Shadow color, Color[1] = Highlight color.
    Ptr<GradientData>   Gradient;       // Gradient, for GradientGlowFilter and GradientBevelFilter (null otherwise)

    
    BlurFilterParams()
        : Mode(0), Passes(1), BlurX(100), BlurY(100), Offset(0,0), Strength(1), Gradient(0)
    {
        Colors[0].SetRGBA(0,0,0,0xFF);
        Colors[1].SetRGBA(0,0,0,0);
    }
    BlurFilterParams(unsigned mode, float x, float y, unsigned passCount = 1,
                     PointF offset = PointF(0,0), Color color = Color(),
                     float strength = 1)
        : Mode(mode), Passes(passCount), BlurX(x), BlurY(y),
          Offset(offset), Strength(strength)
    {
        Colors[0] = color;
        Colors[1].SetRGBA(0,0,0,0);
    }

    BlurFilterParams(unsigned mode, float x, float y,
                     unsigned passCount = 1, float strength = 1)
        : Mode(mode), Passes(passCount), BlurX(x), BlurY(y),
          Offset(0,0), Strength(strength)
    {
        Colors[0].SetRGBA(0,0,0,0xFF);
        Colors[1].SetRGBA(0,0,0,0);
    }

    bool operator==(const BlurFilterParams& b) const
    {
        return (Mode == b.Mode && BlurX == b.BlurX && BlurY == b.BlurY && Passes == b.Passes);
    }

    bool EqualsAll(const BlurFilterParams& b) const 
    {
        return *this == b && Offset == b.Offset && Strength == b.Strength && 
            Colors[0] == b.Colors[0] && Colors[1] == b.Colors[1];
    }

    FilterType GetFilterType() const { return (FilterType)(Mode & Mode_FilterMask); }

    BlurFilterParams& Scale(float x, float y)
    {
        BlurX = Alg::Max<float>(1, BlurX*x);
        BlurY = Alg::Max<float>(1, BlurY*y);
        Offset.x *= x;
        Offset.y *= y;
        return *this;
    }

    // Computes blur/shadow offset based on an angle in radians and distance.
    static PointF CalcOffsetByAngleDistance(float angle, float distance)
    {
        return PointF(cosf(angle) * distance, sinf(angle) * distance);
    }
};

}}

#endif // INC_SF_Render_FilterParams_H
