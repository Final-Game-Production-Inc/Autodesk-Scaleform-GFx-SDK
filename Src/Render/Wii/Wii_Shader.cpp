/**************************************************************************

Filename    :   Wii_Shader.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/Wii/Wii_Shader.h"
#include "Render/Wii/Wii_HAL.h"
#include "Kernel/SF_Debug.h"


namespace Scaleform { namespace Render { namespace Wii {

ShaderManager::ShaderManager()
{
    memset(&SFormats, 0, sizeof(SFormats));
}

void ShaderManager::Initialize(HAL* phal, const ProfileViews* pProfiler)
{
    pHal = phal;
    Profiler = pProfiler;
}

void ShaderManager::SetTexture(Shader& shader, unsigned stage, Texture* ptexture, ImageFillMode fm)
{
    ptexture->ApplyTexture(stage, fm);

    if (shader.NumTex < stage + ptexture->GetTextureStageCount())
        shader.NumTex = stage + ptexture->GetTextureStageCount();
}

static inline void ApplyCxform(Shader* s, unsigned& Stage, GXTevColorArg color, GXTevAlphaArg alpha)
{
    if (Stage == 1 && color == GX_CC_RASC)
        GXSetTevSwapMode((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_SWAP0, GX_TEV_SWAP0);

#ifdef SF_BUILD_DEBUG
    if (color != GX_CC_TEXC && color != GX_CC_RASC && alpha != GX_CC_TEXA && alpha != GX_CC_RASA)
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
#endif

    GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CC_ZERO, color, GX_CC_C1, GX_CC_C0);
    GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_ZERO, alpha, GX_CA_A1, GX_CA_A0);
    GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    Stage++;
}

static inline void ApplyCxformIf(Shader* s, unsigned& Stage, unsigned fillFlags, GXTevColorArg color, GXTevAlphaArg alpha)
{
    if (Stage == 1 && color == GX_CC_RASC)
        GXSetTevSwapMode((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_SWAP0, GX_TEV_SWAP0);

#ifdef SF_BUILD_DEBUG
    if (color != GX_CC_TEXC && color != GX_CC_RASC && alpha != GX_CC_TEXA && alpha != GX_CC_RASA)
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
#endif

    if (fillFlags & FF_Cxform)
    {
        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CC_ZERO, color, GX_CC_C1, GX_CC_C0);
        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_ZERO, alpha, GX_CA_A1, GX_CA_A0);
        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    else
    {
        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, color);
        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, alpha);
        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);

    }
    Stage++;
}

static inline void ApplyFactorAlpha(unsigned& Stage, GXChannelID channel,
                                    GXTevColorArg color = GX_CC_CPREV, GXTevAlphaArg alpha = GX_CA_APREV,
                                    GXTexMapID tex = GX_TEXMAP_NULL, GXTexCoordID tc = GX_TEXCOORD_NULL)
{
    if (Stage == 1)
        GXSetTevSwapMode((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_SWAP0, GX_TEV_SWAP0);

    GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), tc, tex, channel);
    GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, color);
    GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_ZERO, alpha, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    Stage++;
}

static bool IsFillTexAlpha(PrimitiveFill* pfill, int stage = 0)
{
    if (!pfill)
        return 0;
    const Texture* ptex = (const Texture*)pfill->GetTexture(stage);
    return ptex ? ptex->GetTextureFormatMapping()->IsAlpha : 0;
}

Shader ShaderManager::SetFill(PrimitiveFill* pfill, PrimitiveFillType fillType, unsigned fillFlags, unsigned batchType,
                              const VertexFormat* pFormat)
{
    CurShader.BeginPrimitive();

    SF_ASSERT(pFormat->pSysFormat);
    {
        SysVertexFormat* pwiivf = (SysVertexFormat*)pFormat->pSysFormat.GetPtr();
        CurShader.IsComplexMesh = pwiivf->IsComplexMesh;
        if (!pwiivf->IsComplexMesh)
        {
            if (pwiivf->vfn >= 0)
            {
                if (!SFormats[pwiivf->vfn].IsSet)
                {
                    for (int i = 0; i < 8; i++)
                        if (SFormats[pwiivf->vfn].Formats[i])
                            SetVertexAttr(pwiivf->vfn, SFormats[pwiivf->vfn].Formats[i], SFormats[pwiivf->vfn].Formats[4]);
                    SFormats[pwiivf->vfn].IsSet = 1;
                }
            }
            else
            {
                bool hasColor = 0;
                for (const VertexElement* oe = pFormat->pElements; oe->Attribute; oe++)
                {
                    hasColor |= (oe->Attribute & (VET_Usage_Mask|VET_Index_Mask)) == VET_Color;
                    SetVertexAttr(GX_VTXFMT7, oe->Attribute, hasColor);
                }
            }
        }
    }

    unsigned Stage = 0;
    bool     uvtex = 0;
    GXTevColorArg texc, texc1;

    switch (fillType)
    {
    case PrimFill_Mask:
    default:
        {
            GXColor c = { 255,0,255,128};
            GXSetChanMatColor(GX_COLOR0A0, c);
        }

    case PrimFill_None:
    case PrimFill_SolidColor:
        //CurShader.ColorStage = Stage;
        //CurShader.SolidColor = GX_COLOR0A0;

        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        Stage++;
        break;

    case PrimFill_Texture:
        CurShader.NumTex = 1;
        GXSetNumChans(0);
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        texc = IsFillTexAlpha(pfill) ? GX_CC_ONE : GX_CC_TEXC;
        ApplyCxformIf(&CurShader, Stage, fillFlags, texc, GX_CA_TEXA);
        break;

    case PrimFill_UVTexture:
        CurShader.NumTex = 1;
        uvtex = 1;
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

        GXSetNumChans(0);
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        texc = IsFillTexAlpha(pfill) ? GX_CC_ONE : GX_CC_TEXC;
        ApplyCxformIf(&CurShader, Stage, fillFlags, texc, GX_CA_TEXA);
        break;

    case PrimFill_VColor:
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

        //GXSetTevSwapMode((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        ApplyCxformIf(&CurShader, Stage, fillFlags, GX_CC_RASC, GX_CA_RASA);
        break;

    case PrimFill_VColor_EAlpha:
        GXSetNumChans(2);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

        //GXSetTevSwapMode((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

        ApplyCxformIf(&CurShader, Stage, fillFlags, GX_CC_RASC, GX_CA_RASA);
        ApplyFactorAlpha(Stage, GX_COLOR1A1);
        break;

    case PrimFill_Texture_EAlpha:
        CurShader.NumTex = 1;
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

        texc = IsFillTexAlpha(pfill) ? GX_CC_ONE : GX_CC_TEXC;

        if (fillFlags & FF_Cxform)
        {
            GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
            ApplyCxform(&CurShader, Stage, texc, GX_CA_TEXA);
            ApplyFactorAlpha(Stage, GX_COLOR0A0);
        }
        else
            ApplyFactorAlpha(Stage, GX_COLOR0A0, texc, GX_CA_TEXA, GX_TEXMAP0, GX_TEXCOORD0);
        break;

    case PrimFill_Texture_VColor:
        CurShader.NumTex = 1;
        GXSetNumChans(2);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

        texc = IsFillTexAlpha(pfill) ? GX_CC_ONE : GX_CC_TEXC;

        // color in
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        ++Stage;

        // blend tex with color
        SF_ASSERT(GX_TEVSTAGE1 == (GX_TEVSTAGE0 + Stage));
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP1, GX_TEV_SWAP0);
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR1A1);
        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CC_CPREV, texc, GX_CC_RASC, GX_CC_ZERO);
        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_APREV, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        ++Stage;

        if (fillFlags & FF_Cxform)
        {
            ApplyCxform(&CurShader, Stage, GX_CC_CPREV, GX_CA_APREV);
        }
        break;

    case PrimFill_Texture_VColor_EAlpha:
        CurShader.NumTex = 1;
        GXSetNumChans(2);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

        texc = (IsFillTexAlpha(pfill) ? GX_CC_ONE : GX_CC_TEXC);

        // color in
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        ++Stage;

        // blend tex with color
        SF_ASSERT(GX_TEVSTAGE1 == (GX_TEVSTAGE0 + Stage));
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP1, GX_TEV_SWAP0);
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR1A1);
        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CC_CPREV, texc, GX_CC_RASC, GX_CC_ZERO);
        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_APREV, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        ++Stage;

        if (fillFlags & FF_Cxform)
        {
            ApplyCxform(&CurShader, Stage, GX_CC_CPREV, GX_CA_APREV);
        }
        ApplyFactorAlpha(Stage, GX_COLOR1A1);
        break;

    case PrimFill_2Texture:
        CurShader.NumTex = 2;
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

        // tex 0 input
        texc = (pfill && IsFillTexAlpha(pfill) ? GX_CC_ONE : GX_CC_TEXC);
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, texc);
        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        ++Stage;

        // blend tex 1 with tex 0
        SF_ASSERT(GX_TEVSTAGE1 == (GX_TEVSTAGE0 + Stage));
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP1, GX_TEV_SWAP0);
        texc1 = (IsFillTexAlpha(pfill, 1) ? GX_CC_ONE : GX_CC_TEXC);
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), texc1, GX_CC_CPREV, GX_CC_RASC, GX_CC_ZERO);
        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_TEXA, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        ++Stage;

        if (fillFlags & FF_Cxform)
            ApplyCxform(&CurShader, Stage, GX_CC_CPREV, GX_CA_APREV);
        break;

    case PrimFill_2Texture_EAlpha:
        CurShader.NumTex = 2;
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

        // tex 0 input
        texc = (pfill && IsFillTexAlpha(pfill) ? GX_CC_ONE : GX_CC_TEXC);
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, texc);
        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        ++Stage;

        // blend tex 1 with tex 0
        SF_ASSERT(GX_TEVSTAGE1 == (GX_TEVSTAGE0 + Stage));
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP1, GX_TEV_SWAP0);
        texc1 = (IsFillTexAlpha(pfill, 1) ? GX_CC_ONE : GX_CC_TEXC);
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), texc1, GX_CC_CPREV, GX_CC_RASC, GX_CC_ZERO);
        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_TEXA, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        ++Stage;

        if (fillFlags & FF_Cxform)
            ApplyCxform(&CurShader, Stage, GX_CC_CPREV, GX_CA_APREV);

        ApplyFactorAlpha(Stage, GX_COLOR0A0);
        break;

    case PrimFill_UVTextureAlpha_VColor:
        CurShader.NumTex = 1;
        uvtex = 1;
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

        // cxform
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        ApplyCxform(&CurShader, Stage, GX_CC_RASC, GX_CA_RASA);

        // blend tex alpha with color
        GXSetTevSwapMode((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        ++Stage;
        break;
    }

    if (fillFlags & FF_Multiply)
    {
        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + Stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        ++Stage;
    }

    CurShader.NumStages = Stage;
    GXSetNumTevStages(Stage);
    GXSetNumTexGens(CurShader.NumTex);

    if (!uvtex)
    {
        for (int i = 0; i < CurShader.NumTex; i++)
            GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0 + i), GX_TG_MTX2x4, GX_TG_POS, 3 + i * 2);

        CurShader.NumTexMatrix = CurShader.NumTex;
    }

    if (batchType == PrimitiveBatch::DP_Batch)
    {
        CurShader.NumMatrices = 1 + CurShader.NumTexMatrix;
        CurShader.BatchMatrixSize = 3 + 2 * CurShader.NumTexMatrix;
    }

    return CurShader;
}

Shader ShaderManager::SetPrimitiveFill(PrimitiveFill* pfill, unsigned& fillFlags, unsigned batchType, const VertexFormat* pFormat,
                                       unsigned meshCount, const Render::MatrixState* halMatrices, const Primitive::MeshEntry* pmeshes)
{
    const MatrixState* Matrices = (MatrixState*)halMatrices;
    
    PrimitiveFillType fillType = pfill->GetType(); 

    for (unsigned i = 0; i < meshCount; i++)
        // no need to check profiler here because if it sets a cxform it won't depend on the mesh
        if (!pmeshes[i].M.GetCxform().IsIdentity())
        {
            fillFlags |= FF_Cxform;
            break;
        }

    Shader pso = SetFill(pfill, fillType, fillFlags, batchType, pFormat);
    unsigned tmCount = pso.NumTexMatrix;

    if (fillType == PrimFill_SolidColor)
    {
        SetColor(pso, pfill->GetSolidColor());
    }
    else if (fillType >= PrimFill_Texture)
    {     
        Texture*  pt0 = (Texture*)pfill->GetTexture(0);
        SF_ASSERT(pt0);

        unsigned stage = 0;
        SetTexture(pso, stage, pt0, pfill->GetFillMode(0));
        stage += pt0->GetTextureStageCount();

        if ((fillType == PrimFill_2Texture) || (fillType == PrimFill_2Texture_EAlpha))
        {
            Texture* pt1 = (Texture*)pfill->GetTexture(1);
            SetTexture(pso, stage, pt1, pfill->GetFillMode(1));
        }
    }

    if (batchType == PrimitiveBatch::DP_Single)
    {
        SetMatrix(pso, Uniform::SU_mvp, pmeshes[0].pMesh->VertexMatrix, pmeshes[0].M, Matrices);

        for (unsigned tm = 0; tm < tmCount; tm++)
        {
            Matrix2F m(pmeshes[0].pMesh->VertexMatrix);
            m.Append(pmeshes[0].M.GetTextureMatrix(tm));
            SetMatrix(pso, Uniform::SU_texgen, m, tm);
        }
    }
    else
    {
        for (unsigned i = 0; i < meshCount; i++)
        {
            SetMatrix(pso, Uniform::SU_mvp, pmeshes[i].pMesh->VertexMatrix, pmeshes[i].M, Matrices, 0, i);

            for (unsigned tm = 0; tm < tmCount; tm++)
            {
                Matrix2F m(pmeshes[i].pMesh->VertexMatrix);
                m.Append(pmeshes[i].M.GetTextureMatrix(tm));
                SetMatrix(pso, Uniform::SU_texgen, m, tm, i);
            }
        }
    }

    return pso;
}

extern const UInt32 Factors[] = {0, 0xffffff00, 0x000000ff, 0xffffffff};

void ShaderManager::BeginScene()
{
    for (int i = 0; i < 7; i++)
        SFormats[i].IsSet = 0;

    GXSetArray(GX_VA_CLR1, Factors, 4);
}

struct WiiVertexFormat
{
    unsigned Attr[4];
    unsigned NumMatrix;
};

static const WiiVertexFormat WiiVertexFormats[PrimFill_Type_Count] = 
    {{{}, 0},
     /* Mask */              {{VET_Pos, 0}, 1},
     /* Solid */             {{VET_Pos, 0}, 1},
     /* VColor */            {{VET_Pos, VET_Color,                             0}, 1},
     /* VColor EA */         {{VET_Pos, VET_Color, VET_Color | VET_Index1 | 2, 0}, 1},
     /* Texture */           {{VET_Pos,                                        0}, 2},
     /* Texture EA */        {{VET_Pos,            VET_Color | VET_Index1 | 1, 0}, 2},
     /* Texture VColor */    {{VET_Pos, VET_Color, VET_Color | VET_Index2 | 1, 0}, 2},
     /* Texture VColor EA */ {{VET_Pos, VET_Color, VET_Color | VET_Index1 | 2, 0}, 2},
     /* 2Texture */          {{VET_Pos,            VET_Color | VET_Index1 | 1, 0}, 3},
     /* 2Texture EA */       {{VET_Pos,            VET_Color | VET_Index1 | 2, 0}, 3},
     /* TextureUV */         {{VET_Pos,            VET_TexCoord,               0}, 1},
     /* TextureUV A VC    */ {{VET_Pos, VET_Color, VET_TexCoord,               0}, 1}
    };

static int WiiPosComponents[] = {0, 0,        GX_POS_XY, GX_POS_XYZ, 0};
static int WiiTexComponents[] = {0, GX_TEX_S, GX_TEX_ST, 0,          0};

void ShaderManager::SetVertexAttr(int n, int Attr, bool hasColor)
{
    GXAttr usage;
    unsigned count, type, frac = 0;

    switch (Attr & (VET_Usage_Mask|VET_Index_Mask))
    {
    case VET_Pos:                usage = GX_VA_POS;  count = WiiPosComponents[Attr & VET_Components_Mask]; break;
    case VET_Color:              usage = GX_VA_CLR0; count = GX_CLR_RGBA; type = GX_RGBA8; hasColor = 1; break;
    case VET_Color | VET_Index1:
    case VET_Color | VET_Index2: usage = hasColor ? GX_VA_CLR1 : GX_VA_CLR0; count = GX_CLR_RGBA; type = GX_RGBA8; break;
    case VET_TexCoord:           usage = GX_VA_TEX0; count = WiiTexComponents[Attr & VET_Components_Mask]; break;
    default: SF_ASSERT(0);
    }

    if ((Attr & VET_Usage_Mask) != VET_Color)
        switch (Attr & (VET_CompType_Mask))
        {
        case VET_U8N: type = GX_U8; frac = 8; break;
        case VET_U8:  type = GX_U8; break;
        case VET_S16: type = GX_S16; break;
        case VET_U16: type = GX_U16; break;
        case VET_F32: type = GX_F32; break;
        default:      SF_ASSERT(0);
        }

    GXSetVtxAttrFmt((GXVtxFmt)n, usage, (GXCompCnt)count, (GXCompType)type, frac);
}

int ShaderManager::SetVertexArray(GXVtxFmt vf, const VertexFormat* pvf, UByte* pvertexData)
{
    int      vcount = 0;
    int      ncolors = 0, ntexcoord = 0;
    UInt32   vcd0 = 0, vcd1 = 0;

    GXClearVtxDesc();
    for (const VertexElement* pve = pvf->pElements; pve->Attribute; pve++)
    {
        GXAttr usage;
        GXAttrType index = GX_INDEX16;
        unsigned count, type, frac = 0;
        unsigned offset = pve->Offset;

        if ((pve->Attribute & VET_Usage_Mask) == VET_Color)
            switch (pve->Attribute & (VET_CompType_Mask))
            {
                case VET_U8N: type = GX_RGBA8;
                              // Packed factors
                              if (((pve->Attribute | pve[1].Attribute) & (VET_Usage_Mask|VET_Index_Mask|VET_CompType_Mask)) == (VET_Color | VET_U8N | (3 << VET_Index_Shift)))
                              {
                                  pve++;
                                  offset -= 2; // .ba
                              }
                              else if ((pve->Attribute & VET_Index_Mask) == VET_Index1)
                                  offset -= 3; // .a
                              else if ((pve->Attribute & VET_Index_Mask) == VET_Index2)
                                  offset -= 2; // .b
                              break;
                case VET_I8:  index = GX_INDEX8; vcount += 65535; break;
                default:      SF_ASSERT(0);
            }
        else
            switch (pve->Attribute & (VET_CompType_Mask))
            {
                case VET_U8N: type = GX_U8; frac = 8; break;
                case VET_U8:  type = GX_U8; break;
                case VET_S16: type = GX_S16; break;
                case VET_U16: type = GX_U16; break;
                case VET_F32: type = GX_F32; break;
                case VET_I8:  index = GX_INDEX8; vcount += 65535; break;
                default:      SF_ASSERT(0);
            }

        switch (pve->Attribute & (VET_Usage_Mask|VET_Index_Mask))
        {
        case VET_Pos:                usage = GX_VA_POS;  count = WiiPosComponents[pve->Attribute & VET_Components_Mask]; break;
        case VET_Color:              usage = GX_VA_CLR0; count = GX_CLR_RGBA; ncolors++; break;
        case VET_Color | VET_Index1: 
        case VET_Color | VET_Index2: usage = ncolors ? GX_VA_CLR1 : GX_VA_CLR0; ncolors++; count = GX_CLR_RGBA; break;
        case VET_TexCoord:           usage = GX_VA_TEX0; ntexcoord++; count = WiiTexComponents[pve->Attribute & VET_Components_Mask]; break;
        case VET_Instance:           continue;
        default: SF_ASSERT(0);
        }

        GXSetVtxAttrFmt(vf, usage, (GXCompCnt)count, (GXCompType)type, frac);
        if (index == GX_INDEX8)
            GXSetArray(usage, Factors, 4);
        else
            GXSetArray(usage, pvertexData + offset, pvf->Size);
        GXSetVtxDesc(usage, index);
        vcount++;
    }

    return vcount;
}

void    ShaderManager::MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                       const VertexFormat** single,
                                       const VertexFormat** batch, const VertexFormat** instanced, unsigned meshType)
{
    const WiiVertexFormat* psvf = &WiiVertexFormats[fill];
    VertexElement          outf[8];
    unsigned               size = 0, indexsize = 0, extras = 0;
    int                    j = 1;

    for (const unsigned *attr = psvf->Attr; *attr; attr++)
    {
        if (meshType == MeshCacheItem::Mesh_Complex)
        {
            if ((*attr & (VET_Usage_Mask | VET_Index_Mask | VET_Components_Mask)) == (VET_Color | VET_Index1 | 2))
            {
                outf[j].Offset = size;
                outf[j].Attribute = VET_T0Weight8;
                j++;
                outf[j].Offset = size+1;
                outf[j].Attribute = VET_FactorAlpha8;
                j++;
                size += 2;
                continue;
            }
        }
        else
        {
            // regular mesh, processed with PreparePrimitive, use indexed factors
            if ((*attr & (VET_Usage_Mask | VET_Index_Mask | VET_Components_Mask)) == (VET_Color | VET_Index1 | 2))
            {
                outf[j].Offset = indexsize;
                outf[j].Attribute = VET_IndexedFactors;
                j++;
                indexsize++;
                extras++;
                continue;
            }
            else if ((*attr & (VET_Usage_Mask | VET_Index_Mask)) == (VET_Color | VET_Index1) ||
                     (*attr & (VET_Usage_Mask | VET_Index_Mask)) == (VET_Color | VET_Index2))
            {
                outf[j].Offset = indexsize;
                outf[j].Attribute = VET_I8 | VET_Color | (*attr & (VET_Usage_Mask | VET_Index_Mask | VET_Components_Mask));
                j++;
                indexsize++;
                extras++;
                continue;
            }
        }

        const VertexElement* pv = sourceFormat->GetElement(*attr & (VET_Usage_Mask|VET_Index_Mask), VET_Usage_Mask|VET_Index_Mask);
        if (!pv)
        {
            continue;
            //*batch = *single = *instanced = NULL;
            //return;
        }
        if ((pv->Attribute & VET_Components_Mask) == 0)
            continue;

        outf[j] = *pv;
        outf[j].Offset = size;
        indexsize += 2;

        SF_ASSERT((outf[j].Attribute & VET_Components_Mask) > 0 && (outf[j].Attribute & VET_Components_Mask) <= 4);
        size += outf[j].Size();
        size = (size + pv->CompSize() - 1) & ~(pv->CompSize() - 1);
        j++;
    }
    outf[j].Attribute = VET_None;
    outf[j].Offset = 0;
    VertexFormat* newvf;
    *single = newvf = GetVertexFormat(outf+1, j, size);
    if (!newvf->pSysFormat)
        SetSysVertexFormat(newvf, indexsize, 0, meshType == MeshCacheItem::Mesh_Complex, extras);

    outf[0].Attribute = VET_Instance | VET_I8 | psvf->NumMatrix;
    outf[0].Offset = 0;
    for (int i = 1; i < j; i++)
        if ((outf[i].Attribute & VET_CompType_Mask) == VET_I8)
            outf[i].Offset += psvf->NumMatrix;
/*
    if (j > 2)
    {
        *batch = newvf = GetVertexFormat(outf, j+1, size);
        if (!newvf->pSysFormat)
        {
            const SysVertexFormat* pwiivf = (const SysVertexFormat*)(*single)->pSysFormat.GetPtr();
            newvf->pSysFormat = (Render::SystemVertexFormat*)
                SF_NEW SysVertexFormat(pwiivf->vfn, psvf->NumMatrix, indexsize+psvf->NumMatrix, meshType == MeshCacheItem::Mesh_Complex, 0);
        }
    }
    else
  */      *batch = 0;
    *instanced = 0;
}

void ShaderManager::SetSysVertexFormat(VertexFormat* newvf, unsigned indexsize, unsigned numMatrices, bool iscomplex, unsigned extras)
{
    if (!iscomplex)
    for (int i = 0; i < 7; i++)
    {
        StaticVertexFormat *pvd = &SFormats[i];
        bool found = 0;
        const VertexElement* oe;

        for (oe = newvf->pElements; oe->Attribute; oe++)
        {
            int ve = ((oe->Attribute & VET_Usage_Mask) >> (VET_Usage_Shift-1)) | ((oe->Attribute & VET_Index_Mask) >> VET_Index_Shift);
            if (pvd->Formats[ve] && pvd->Formats[ve] != oe->Attribute)
                break;
        }

        if (!oe->Attribute)
        {
            bool     hasColor = 0;

            for (const VertexElement* oe = newvf->pElements; oe->Attribute; oe++)
            {
                int ve = ((oe->Attribute & VET_Usage_Mask) >> (VET_Usage_Shift-1)) | ((oe->Attribute & VET_Index_Mask) >> VET_Index_Shift);
                if (pvd->Formats[ve] != oe->Attribute)
                {
                    SF_ASSERT(pvd->Formats[ve] == 0);
                    pvd->Formats[ve] = oe->Attribute;
                }
            }
            pvd->IsSet = 0;

            newvf->pSysFormat = (Render::SystemVertexFormat*)SF_NEW SysVertexFormat(i, numMatrices, indexsize, iscomplex, extras);
            return;
        }
    }

    newvf->pSysFormat = (Render::SystemVertexFormat*)SF_NEW SysVertexFormat(-1, numMatrices, indexsize, iscomplex, extras);
}

}}}
