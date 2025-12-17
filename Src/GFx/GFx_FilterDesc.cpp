/**************************************************************************

Filename    :   GFx_FilterDesc.cpp
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_Stream.h"
#include "GFx/GFx_FilterDesc.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace GFx {

using Render::BlurFilterParams;

// ***** Tag Loaders implementation

/*
    struct swf_filter_drop_shadow 
    {
        swf_filter_type     f_type;     // 0
        swf_rgba            f_rgba;
        signed long fixed   f_blur_horizontal;
        signed long fixed   f_blur_vertical;
        signed long fixed   f_radian_angle;
        signed long fixed   f_distance;
        signed short fixed  f_strength;
        unsigned            f_inner_shadow     : 1;
        unsigned            f_knock_out        : 1;
        unsigned            f_composite_source : 1;
        unsigned            f_reserved1        : 1;
        unsigned            f_reserved2        : 4;
    };

    struct swf_filter_blur 
    {
        swf_filter_type     f_type;     // 1 
        unsigned long fixed f_blur_horizontal;
        unsigned long fixed f_blur_vertical;
        unsigned            f_passes   : 5;
        unsigned            f_reserved : 3;
    };

    struct swf_filter_glow 
    {
        swf_filter_type     f_type;     // 2
        swf_rgba            f_rgba;
        signed long fixed   f_blur_horizontal;
        signed long fixed   f_blur_vertical;
        signed short fixed  f_strength;
        unsigned            f_inner_shadow     : 1;
        unsigned            f_knock_out        : 1;
        unsigned            f_composite_source : 1;
        unsigned            f_reserved1        : 1;
        unsigned            f_reserved2        : 4;
    };
*/


//------------------------------------------------------------------------
// static inline UInt8 GFx_Fixed1616toFixed44U(UInt32 v)
// {
//     unsigned vi = v >> 16;
//     if (vi > 15) vi = 15;
//     return UInt8((vi << 4) | ((v >> 12) & 0xF));
// }

//------------------------------------------------------------------------
// static inline UInt8 GFx_Fixed88toFixed44U(UInt16 v)
// {
//     unsigned vi = v >> 8;
//     if (vi > 15) vi = 15;
//     return UInt8((vi << 4) | ((v >> 4) & 0xF));
// }


//------------------------------------------------------------------------
static inline float GFx_Fixed1616toFloat(UInt32 v)
{
    return (float(v) / 65536.0f);
}

//------------------------------------------------------------------------
static inline float GFx_Fixed88toFloat(UInt16 v)
{
    return (float(v) / 256.0f);
}

//------------------------------------------------------------------------

enum FlashFilterType
{
    FlashFilter_DropShadow   = 0,
    FlashFilter_Blur         = 1,
    FlashFilter_Glow         = 2,
    FlashFilter_Bevel        = 3,
    FlashFilter_GradientGlow = 4,
    FlashFilter_Convolution  = 5,
    FlashFilter_ColorMatrix  = 6,
    FlashFilter_GradientBevel= 7
};


enum BlurFilterReaderConstants 
{
    BFR_Color0          = 0x01,
    BFR_Color1          = 0x02,
    BFR_AngleDistance   = 0x04,
    BFR_Strength        = 0x08,
    BFR_Gradient        = 0x10,
};

template <class T>
void ReadBlurFilter(T* ps, BlurFilterParams* params,
                    float* angle, float* distance,
                    unsigned readConstants, Render::FilterType type, unsigned passesMask)
{
    if (readConstants & BFR_Gradient)
    {
        UInt16 numGradientEntries = ps->ReadU8();
        Ptr<Render::GradientData> gradData = *SF_NEW Render::GradientData(Render::GradientLinear, numGradientEntries, false);
        for (unsigned gradent = 0; gradent < numGradientEntries; ++gradent)
        {
            Render::GradientRecord& record = gradData->At(static_cast<unsigned>(gradent));
            Color c;
            c.SetRed(ps->ReadU8());
            c.SetGreen(ps->ReadU8());
            c.SetBlue(ps->ReadU8());
            c.SetAlpha(ps->ReadU8());
            record.ColorV = c;
        }

        for (unsigned gradent = 0; gradent < numGradientEntries; ++gradent)
        {
            UByte ratio = ps->ReadU8();
            Render::GradientRecord& record = gradData->At(static_cast<unsigned>(gradent));
            record.Ratio = (UByte)ratio;
        }
        params->Gradient = gradData;
    }

    if (readConstants & BFR_Color0)
    {
        ps->ReadRgba(&params->Colors[0]);
        if (readConstants & BFR_Color1)
            ps->ReadRgba(&params->Colors[1]);
    }
    // Our renderer's Blur and Distance filter arguments use source-textfield
    // coordinate system, which is in twips for SWF.
    params->BlurX = PixelsToTwips(GFx::GFx_Fixed1616toFloat(ps->ReadU32()));
    params->BlurY = PixelsToTwips(GFx::GFx_Fixed1616toFloat(ps->ReadU32()));

    if (readConstants & BFR_AngleDistance)
    {
        *angle    = GFx::GFx_Fixed1616toFloat(ps->ReadU32());
        *distance = GFx::GFx_Fixed1616toFloat(ps->ReadU32());
    }
    if (readConstants & BFR_Strength)    
        params->Strength = GFx::GFx_Fixed88toFloat(ps->ReadU16());
    
    unsigned f = ps->ReadU8();
    unsigned mode = 0;

    // Decode filter flags and pass count. These have slightly different
    // layouts based on filter, which we control here through passesMask.
    
    if (passesMask == 0xF8)
    {        
        params->Passes = f >> 3;
    }
    else
    {
        if  (f & 0x80)       mode |= BlurFilterParams::Mode_Inner;
        if  (f & 0x40)       mode |= BlurFilterParams::Mode_Knockout;
        if ((f & 0x20) == 0) mode |= BlurFilterParams::Mode_HideObject;

        if (passesMask <= 0xF)
        {
            if (f & 0x10)
                mode |= BlurFilterParams::Mode_Highlight;
        }
        params->Passes = f & passesMask;
    }

    params->Mode = type | mode;
}



// Helper function used to load filters.
//------------------------------------------------------------------------

template <class T>
unsigned LoadFilters(T* ps, Render::FilterSet* filters)
{
    UByte    filterCount = ps->ReadU8(); 
    unsigned numFilters = 0;
    float    angle = 0.0f, distance = 0.0f;
    MemoryHeap* filtersHeap = (filters) ? Memory::GetHeapByAddress(filters) : Memory::GetGlobalHeap();

    while(filterCount--)
    {
        FlashFilterType  filterType = (FlashFilterType)ps->ReadU8();
        unsigned         numBytes = 0;
        Ptr<Render::Filter>      filter = 0;
        BlurFilterParams params;

        switch(filterType)
        {
        case FlashFilter_DropShadow: // 23 bytes
            {
                ReadBlurFilter(ps, &params, &angle, &distance,
                    BFR_Color0|BFR_AngleDistance|BFR_Strength,
                    Render::Filter_Shadow, 0x1F);
                filter = *SF_HEAP_NEW(filtersHeap) Render::ShadowFilter(params, angle, distance);
            }
            break;

        case FlashFilter_Blur: // 9 bytes
            ReadBlurFilter(ps, &params, 0, 0,
                           0, Render::Filter_Blur, 0xF8);
            filter = *SF_HEAP_NEW(filtersHeap) Render::BlurFilter(params);
            break;

        case FlashFilter_Glow: // 15 bytes
            //blurFilter = true;
            ReadBlurFilter(ps, &params, 0, 0,
                           BFR_Color0|BFR_Strength, Render::Filter_Glow, 0x1F);
            filter = *SF_HEAP_NEW(filtersHeap) Render::GlowFilter(params);
            break;

        case FlashFilter_Bevel:
            //blurFilter = hasAngleDistance = true;
            ReadBlurFilter(ps, &params, &angle, &distance,
                           BFR_Color0|BFR_Color1|BFR_AngleDistance|BFR_Strength,
                           Render::Filter_Bevel, 0xF);
            filter = *SF_HEAP_NEW(filtersHeap) Render::BevelFilter(params, angle, distance);
            break;

        case FlashFilter_GradientGlow:
        case FlashFilter_GradientBevel:
        {           
            Render::FilterType type = filterType == FlashFilter_GradientGlow ? Render::Filter_GradientGlow : Render::Filter_GradientBevel;
            ReadBlurFilter(ps, &params, &angle, &distance,
                           BFR_AngleDistance|BFR_Strength|BFR_Gradient,
                           type, 0xF);
            filter = *SF_HEAP_NEW(filtersHeap) Render::GradientFilter(type, params, angle, distance);
            break;
        }

        case FlashFilter_Convolution:
            {
                unsigned cols = ps->ReadU8();
                unsigned rows = ps->ReadU8();
                numBytes = 4 + 4 + 4*cols*rows + 4 + 1;
            }
            break;

        case FlashFilter_ColorMatrix:
            {
                static const UByte Index[] = {0,1,2,3,16, 4,5,6,7,17, 8,9,10,11,18, 12,13,14,15,19};
                Ptr<Render::ColorMatrixFilter> cmFilter = *SF_HEAP_NEW(filtersHeap) Render::ColorMatrixFilter;
                filter = cmFilter;

                unsigned i;
                for (i = 0; i < 20; i++)
                    (*cmFilter)[Index[i]] = ps->ReadFloat();
                for (i = 16; i < 20; i++)
                    (*cmFilter)[i] *= 1.f/255.f;
                numFilters++;
            }
            break;
        }

        // If we had  a blur filter, add it to list.
        if (filters && filter)
        {
            filters->AddFilter(filter);
            numFilters++;
        }

        // Skip filter or the rest of the filter
        while(numBytes--)
            ps->ReadU8();
    }

    return numFilters;
}
// Force concrete definitions inside .cpp
template unsigned LoadFilters(Stream* ps, Render::FilterSet* filters);
template unsigned LoadFilters(StreamContext* ps, Render::FilterSet* filters);


}} // Scaleform::GFx
