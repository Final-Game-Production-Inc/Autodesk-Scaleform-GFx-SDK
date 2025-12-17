/**************************************************************************

Filename    :   PS3_Shader.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/PS3/PS3_Shader.h"
#include "Render/PS3/PS3_HAL.h"

namespace Scaleform { namespace Render { namespace PS3 {


bool VertexShader::Init(const VertexShaderDesc* pd)
{
    pDesc = pd;
    Prog = (CGprogram)pDesc->pBinary;
    cellGcmCgInitProgram(Prog);

    size_t ucsize;
    cellGcmCgGetUCode(Prog, &UCode, &ucsize);

    memset(Attributes, -1, sizeof Attributes);
    for (int i = 0; i < pDesc->NumAttribs; i++)
    {
        CGparameter attr = cellGcmCgGetNamedParameter(Prog, pDesc->Attributes[i].Name);
        if (attr)
            Attributes[i] = (CGresource)(cellGcmCgGetParameterResource(Prog, attr) - CG_ATTR0);
        else
            Attributes[i] = 17;
    }

    memset(Uniforms, 0, sizeof(Uniforms));
    for (int i = 0; i < PS3::Uniform::SU_Count; i++)
        if (pDesc->Uniforms[i].Location >= 0)
        {
            CGparameter uvar = cellGcmCgGetNamedParameter(Prog, ShaderUniformNames[i]);
            if (!uvar)
            {
                char arrayname[128];
                strcpy(arrayname, ShaderUniformNames[i]);
                strcat(arrayname, "[0]");
                uvar = cellGcmCgGetNamedParameter(Prog, arrayname);
            }
            if (uvar)
                Uniforms[i] = cellGcmCgGetParameterResourceIndex(Prog, uvar);
            else
                Uniforms[i] = 0x10000;
        }

    return true;
}

bool FragShader::Init(const FragShaderDesc* pd, MemoryManager* pmm)
{
    if (pMemManager && UCode)
        pMemManager->Free(UCode, UCodeSize);

    pMemManager = pmm;
    pDesc = pd;
    Prog = (CGprogram)pDesc->pBinary;
    cellGcmCgInitProgram(Prog);

    memset(Uniforms, 0, sizeof(Uniforms));

    for (int i = 0; i < PS3::Uniform::SU_Count; i++)
    {
        if (pDesc->Uniforms[i].Location >= 0)
        {
            CGparameter uvar = cellGcmCgGetNamedParameter(Prog, ShaderUniformNames[i]);
            if (!uvar)
            {
                char arrayname[128];
                strcpy(arrayname, ShaderUniformNames[i]);
                strcat(arrayname, "[0]");
                uvar = cellGcmCgGetNamedParameter(Prog, arrayname);
            }
            Uniforms[i] = uvar;

            if ( uvar && cellGcmCgGetParameterType(Prog, Uniforms[i]) == CG_SAMPLER2D)
            {
                SF_DEBUG_ASSERT(cellGcmCgGetParameterResource(Prog, Uniforms[i]) - CG_TEXUNIT0 == pDesc->Uniforms[i].Location,
                    "Texture resource index and location do not match.");
            }
        }
    }

    void *uc;
    cellGcmCgGetUCode(Prog, &uc, &UCodeSize);

    UCode = pmm->Alloc(UCodeSize, 128, Memory_PS3_RSX);
    if (UCode)
    {
        cellGcmAddressToOffset(UCode, &Offset);
        memcpy(UCode, uc, UCodeSize);
    }
    return UCode != 0;
}

FragShader::~FragShader()
{
    Shutdown();
}

void FragShader::Shutdown()
{
    if (pMemManager && UCode)
    {
        pMemManager->Free(UCode, UCodeSize);
        pMemManager = 0;
        UCode       = 0;
        UCodeSize   = 0;
    }
}


bool ShaderInterface::SetStaticShader(ShaderDesc::ShaderType shader, const VertexFormat*)
{
    VertexShaderDesc::ShaderIndex vsIndex = VertexShaderDesc::GetShaderIndex(shader);
    FragShaderDesc::ShaderIndex psIndex = FragShaderDesc::GetShaderIndex(shader);

    CurShaders.pVS    = &pHal->StaticVShaders[vsIndex];
    CurShaders.pVDesc = CurShaders.pVS->pDesc;
    CurShaders.pFS    = &pHal->StaticFShaders[psIndex];
    CurShaders.pFDesc = CurShaders.pFS->pDesc;
    return true;
}

void ShaderInterface::Finish(unsigned meshCount)
{
    bool paramsSet = false;

    for (int i = 0; i < Uniform::SU_Count; i++)
        if (UniformSet[i])
        {
            if (CurShaders.pFS->Uniforms[i])
            {
                SF_ASSERT(CurShaders.pFDesc->Uniforms[i].Location >= 0);
                cellGcmSetFragmentProgramParameter(pHal->GcmCtx, CurShaders.pFS->Prog, CurShaders.pFS->Uniforms[i],
                    UniformData + CurShaders.pFDesc->Uniforms[i].ShadowOffset, CurShaders.pFS->Offset);
                paramsSet = true;
            }
            else
            {
                unsigned size;
                if (CurShaders.pVDesc->Uniforms[i].BatchSize > 0)
                    size = meshCount * CurShaders.pVDesc->Uniforms[i].BatchSize * CurShaders.pVDesc->Uniforms[i].ElementSize >> 2;
                else
                    size = CurShaders.pVDesc->Uniforms[i].Size >> 2;

                cellGcmSetVertexProgramParameterBlock(pHal->GcmCtx, CurShaders.pVS->Uniforms[i], size,
                    UniformData + CurShaders.pVDesc->Uniforms[i].ShadowOffset);
            }
        }

        memset(UniformSet, 0, Uniform::SU_Count);

        if (pLastVS != CurShaders.pVS)
        {
            cellGcmSetVertexProgram(pHal->GcmCtx, CurShaders.pVS->Prog, CurShaders.pVS->UCode);
            pLastVS = CurShaders.pVS;
        }

        if (paramsSet || pLastFS != CurShaders.pFS)
        {
            cellGcmSetUpdateFragmentProgramParameter(pHal->GcmCtx, CurShaders.pFS->Offset);
            cellGcmSetFragmentProgram(pHal->GcmCtx, CurShaders.pFS->Prog, CurShaders.pFS->Offset);
            pLastFS = CurShaders.pFS;
        }
}

void ShaderInterface::SetTexture(ShaderPair shaders, unsigned var, Render::Texture* ptex, ImageFillMode fm, unsigned index)
{
    PS3::Texture* ptexture = (PS3::Texture*)ptex;
    const FragShaderDesc* sd = shaders.pFDesc;

    if (ptexture->NeedsCacheFlush)
    {
        cellGcmSetInvalidateTextureCache(pHal->GcmCtx, CELL_GCM_INVALIDATE_TEXTURE);
        ptexture->NeedsCacheFlush = false;
    }

    for (unsigned plane = 0; plane < ImageData::GetFormatPlaneCount(ptexture->GetImageFormat()); plane++)
    {
        int stageIndex = sd->Uniforms[var].Location;
        SF_DEBUG_ASSERT2(stageIndex >= 0, "Texture parameter %s has no usage in the fragment shader %d.", 
            ShaderUniformNames[var], sd->Type);
        stageIndex += index;
        stageIndex += plane;

        cellGcmSetTexture(pHal->GcmCtx, stageIndex, &ptexture->pTextures[plane].Tex);

        if (fm.GetWrapMode() == Wrap_Clamp)
            cellGcmSetTextureAddress(pHal->GcmCtx, stageIndex, CELL_GCM_TEXTURE_CLAMP_TO_EDGE, CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
                                     CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
                                     CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL, CELL_GCM_TEXTURE_ZFUNC_NEVER, 0);
        else
            cellGcmSetTextureAddress(pHal->GcmCtx, stageIndex, CELL_GCM_TEXTURE_WRAP, CELL_GCM_TEXTURE_WRAP,
                                     CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
                                     CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL, CELL_GCM_TEXTURE_ZFUNC_NEVER, 0);

        cellGcmSetTextureControl(pHal->GcmCtx, stageIndex, CELL_GCM_TRUE, 0, 12 << 8, CELL_GCM_TEXTURE_MAX_ANISO_1);

        bool useMipmaps = ptexture->pTextures[plane].Tex.mipmap > 1;

        // Lod bias is 1:4:8 signed fixed point; -0.5 = 0x1f80
        cellGcmSetTextureFilter(pHal->GcmCtx, stageIndex, useMipmaps ? 0x1f80 : 0, (fm.GetSampleMode() == Sample_Point)
            ? CELL_GCM_TEXTURE_NEAREST : (useMipmaps ? CELL_GCM_TEXTURE_LINEAR_LINEAR : CELL_GCM_TEXTURE_LINEAR),
            (fm.GetSampleMode() == Sample_Point) ? CELL_GCM_TEXTURE_NEAREST : CELL_GCM_TEXTURE_LINEAR,
            CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
    }
}

}}}
