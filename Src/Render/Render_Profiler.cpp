/**********************************************************************

PublicHeader:   Render
Filename    :   Render_Profiler.cpp
Content     :   
Created     :   2012/11/30
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render/Render_Profiler.h"
#include "Render/Render_HAL.h"

namespace Scaleform { namespace Render {

#if defined(SF_RENDERER_PROFILE)

ProfileModifierOverdraw::ProfileModifierOverdraw()
{
    memset(DrawModeCx, 0, sizeof(DrawModeCx));

    // Fill out the color transforms, based on the drawmode.
    DrawModeCx[DrawMode_Fill].M[1][1]       = 1.0f;       // Fills are green
    DrawModeCx[DrawMode_Fill].M[1][3]       = 1/16.0f;    // 1/16th of full intensity
    DrawModeCx[DrawMode_Mask].M[1][0]       = 1.0f;       // Masks are red
    DrawModeCx[DrawMode_Mask].M[1][3]       = 1/4.0f;     // 1/4th of full intensity  (make them more prominent)
    DrawModeCx[DrawMode_CachedPrim].M[1][2] = 1.0f;       // Filters/cached Blends are blue
    DrawModeCx[DrawMode_CachedPrim].M[1][3] = 1/8.0f;     // 1/8th of full intensity (make them more prominent)

    DrawModeCx[DrawMode_Clear].M[1][0]  = 0.5f;      // Clears are gray
    DrawModeCx[DrawMode_Clear].M[1][1]  = 0.5f;
    DrawModeCx[DrawMode_Clear].M[1][2]  = 0.5f;
    DrawModeCx[DrawMode_Clear].M[1][3]  = 8/255.0f;  // 1/32nd of full intensity
}

Color ProfileModifierOverdraw::GetClearColor(DrawModeType, Color) const
{
    // Clears always clear black with full alpha.
    return Color(0,0,0,255);
}

BlendMode ProfileModifierOverdraw::GetBlendMode(DrawModeType mode, BlendMode blend) const
{
    // Always do additive blending in overdraw mode, except for clears.
    return mode == DrawMode_Clear ? blend : Blend_Add;   
}

PrimitiveFillType ProfileModifierOverdraw::GetFillType(DrawModeType, PrimitiveFillType fill) const
{
    // Make mask clear rectangles render as solid colors.
    return (fill == PrimFill_Mask) ? PrimFill_SolidColor : fill;
}
Color ProfileModifierOverdraw::GetColor(DrawModeType mode, Color color) const
{
    // Transform the input color by the current Cxform.
    return GetCxform(mode, Cxform::Identity).Transform(color);
}

bool ProfileModifierOverdraw::IsCxformChanged(DrawModeType) const
{
    // All DrawModes have a non-identity Cxform.
    return true;
}

Cxform ProfileModifierOverdraw::GetCxform(DrawModeType mode, const Cxform&) const
{
    return DrawModeCx[mode];
}

// *** ProfileModifierBatch
ProfileModifierBatch::ProfileModifierBatch() :
    ProfileModifier(),
    NextBatchColor(0),
    SceneBatchCount(0),
    MaskStencilRender(false),
    HighlightBatch(-1)
{

}

Color ProfileModifierBatch::GetClearColor(DrawModeType, Color) const
{
    // Clears always clear black with full alpha.
    return Color(0,0,0,255);
}

PrimitiveFillType ProfileModifierBatch::GetFillType(DrawModeType, PrimitiveFillType fill) const
{
    return (fill == PrimFill_Mask) ? PrimFill_SolidColor : fill;
}

Color ProfileModifierBatch::GetColor(DrawModeType mode, Color color) const
{
    return GetCxform(mode, Cxform::Identity).Transform(color);    
}

bool ProfileModifierBatch::IsCxformChanged(DrawModeType) const
{
    return true;
}

Cxform ProfileModifierBatch::GetCxform(DrawModeType, const Cxform&) const
{
    return Cxform(NextBatchColor);
}

void ProfileModifierBatch::SetBatch(DrawModeType type, HAL*, Primitive* prim, unsigned index)
{
    setBatchHelper(type, prim, index);
}

void ProfileModifierBatch::SetBatch(DrawModeType type, HAL*, ComplexMesh* prim, unsigned index)
{
    setBatchHelper(type, prim, index);
}

bool ProfileModifierBatch::ShouldDrawMaskVisible() const
{
    // If the mask primitive is the highlighted batch, then actually draw it.
    return HighlightBatch == SceneBatchCount;
}

void ProfileModifierBatch::BeginScene()
{
    SceneBatchCount = 0;
}

void ProfileModifierBatch::setBatchHelper( DrawModeType type, void* prim, unsigned index )
{
    if (HighlightBatch == SceneBatchCount)
    {
        if (type != DrawMode_Mask)
            NextBatchColor = Color(0, 255, 0, 192); // Highlighted batch = green.
        else
            NextBatchColor = Color(255, 0, 0, 192); // Highlighted mask batch = red.
    }
    else
    {
        NextBatchColor = ProfileViews::GetColorForBatch((UPInt)prim, index);
        if (HighlightBatch >= 0)
            NextBatchColor.SetAlpha(16); // Vastly reduce alpha of non-highlighted batches.
    }

    // Trickery to deal with masks being highlighted.
    if (MaskStencilRender)
    {
        MaskStencilRender = false;
        return; // don't count this as an actual batch.
    }
    if (type == DrawMode_Mask && HighlightBatch == SceneBatchCount)
    {
        // Next render will be the stencil-only version of the mask
        MaskStencilRender = true;
    }
    SceneBatchCount++;
}

// *** ProfileModifierBlending
Color ProfileModifierBlending::GetClearColor(DrawModeType, Color) const
{
    // Clears always clear black with full alpha.
    return Color(0,0,0,255);
}

BlendMode ProfileModifierBlending::GetBlendMode(DrawModeType mode, BlendMode blend) const
{
    // Always do additive blending, except for clears.
    return mode == DrawMode_Clear ? blend : Blend_Add;   
}

PrimitiveFillType ProfileModifierBlending::GetFillType(DrawModeType, PrimitiveFillType fill) const
{
    return (fill == PrimFill_Mask) ? PrimFill_SolidColor : fill;
}

Color ProfileModifierBlending::GetColor(DrawModeType mode, Color color) const
{
    // Transform the input color by the current Cxform.
    return GetCxform(mode, Cxform::Identity).Transform(color);
}

bool ProfileModifierBlending::IsCxformChanged(DrawModeType) const
{
    return true;
}

Cxform ProfileModifierBlending::GetCxform(DrawModeType, const Cxform&) const
{
    return (FillFlags & FF_Blending) ? Cxform(Color(0,255,0,32)) : Cxform(Color(255,0,0,32));
}

// *** ProfileModifierTDensity
Color ProfileModifierTDensity::GetClearColor(DrawModeType, Color) const
{
    // Clears always clear black with full alpha.
    return Color(0,0,0,255);
}

BlendMode ProfileModifierTDensity::GetBlendMode(DrawModeType mode, BlendMode blend) const
{
    // Always do no blending - blending between HSV colors looks confusing.
    return mode == DrawMode_Clear ? blend : Blend_None;
}

Color ProfileModifierTDensity::GetColor(DrawModeType mode, Color color) const
{
    // Transform the input color by the current Cxform.
    return GetCxform(mode, Cxform::Identity).Transform(color);
}

bool ProfileModifierTDensity::IsCxformChanged(DrawModeType) const
{
    return true;
}

Cxform ProfileModifierTDensity::GetCxform(DrawModeType, const Cxform& cxin) const
{
    // Anything that uses a Cxform, is not using a texture. 'fade' these things to blue.
    Cxform cx(Color(0, 0, 255, 32));
    cx.Prepend(cxin);
    return cx;
}

void ProfileModifierTDensity::SetBatch(DrawModeType, HAL* phal, Primitive* prim, unsigned index)
{
    SF_UNUSED(index);
    if (!prim || !prim->pFill)
        return;
    setUniforms(phal, prim->pFill);
}

void ProfileModifierTDensity::SetBatch(DrawModeType, HAL* phal, ComplexMesh* prim, unsigned index)
{
    if (!prim || prim->GetFillRecordCount() < index)
        return;
    const ComplexMesh::FillRecord& fillRecord = prim->GetFillRecords()[index];
    setUniforms(phal, fillRecord.pFill);
}

void ProfileModifierTDensity::setUniforms(HAL* phal, PrimitiveFill* pfill)
{
    // Find the largest texture in the fill.
    unsigned textureCount = pfill->GetTextureCount();
    ImageSize maxImageSize(0,0);
    unsigned  mipLevels = 0;
    for (unsigned tex = 0; tex < textureCount; ++tex)
    {
        Texture* ptex = pfill->GetTexture(tex);
        ImageSize texImageSize = ptex->GetTextureSize();
        if (texImageSize.Area() > maxImageSize.Area())
        {
            maxImageSize = texImageSize;
            mipLevels    = ptex->GetMipmapCount();
        }
    }

    if (textureCount > 0 )
    {
        Size<float> floatTextureSize = maxImageSize;
        float floatMips = (float)mipLevels;
        phal->profilerApplyUniform(ProfileUniform_TextureSize, 2, &floatTextureSize.Width);
        phal->profilerApplyUniform(ProfileUniform_TextureMips, 2, &floatMips);
    }
}


// *** ProfileViews
const ProfileModifier&  ProfileViews::GetCurrentProfileModifier() const
{
    // If the mode is not available, return the default mode.
    if ((AvailableModes & (1<<ProfileMode)) == 0)
        return PMDefault;

    switch(ProfileMode)
    {
    default:
    case Profile_None:
        return PMDefault;
    case Profile_Overdraw:
        return PMOverdraw;
    case ProfilerModes::Profile_Batch:
        return PMBatch;
    case Profile_Blending:
        return PMBlending;
    case Profile_TextureDensity:
        return PMTDensity;
    }
}

ProfileModifier&  ProfileViews::GetCurrentProfileModifier()
{
    // If the mode is not available, return the default mode.
    if ((AvailableModes & (1<<ProfileMode)) == 0)
        return PMDefault;

    switch(ProfileMode)
    {
    default:
    case Profile_None:
        return PMDefault;
    case Profile_Overdraw:
        return PMOverdraw;
    case ProfilerModes::Profile_Batch:
        return PMBatch;
    case Profile_Blending:
        return PMBlending;
    case Profile_TextureDensity:
        return PMTDensity;
    }
}


// *** ProfileViews
Color ProfileViews::GetColorForBatch( UPInt base, unsigned index )
{
    UPInt key = base ^ (base << 4) >> index;
    float hue = (key & 0x3FF) / 1023.f;

    // PPS: For analyzing covariance
    // printf("%f,\t%f,\t%lu,\t%u\n", key / (float)(((UInt64)0x1 << (sizeof(key) * 8)) - 1), hue, base, index);

    Color batchColor;
    batchColor.SetHSV(hue, 0.8f, 0.9f);
    batchColor.SetAlpha(192);

    return batchColor;
}
void ProfileViews::SetProfileFlag( unsigned flag, bool state )
{
    if (state)
        NextProfileFlags |= flag;
    else
        NextProfileFlags &= ~flag;
}

void ProfileViews::BeginScene(HAL*)
{
    ProfileMode  = NextProfileMode;
    ProfileFlags = NextProfileFlags;

    PMBatch.SetHighlightedBatch(HighlightBatch);
    GetCurrentProfileModifier().BeginScene();
}

#endif // SF_RENDERER_PROFILE

}}; // Scaleform::Render

