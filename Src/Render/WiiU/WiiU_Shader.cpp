/**************************************************************************

Filename    :   WiiU_Shader.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_SysFile.h"
#include "Render/WiiU/WiiU_Shader.h"
#include "Render/WiiU/WiiU_HAL.h"

#include <cafe/gfd.h>


namespace Scaleform { namespace Render { namespace WiiU {


bool ShaderInterface::Initialize(Render::MemoryManager* pmemmgr)
{
    pMemManager = pmemmgr;

    for (unsigned i = 0; i < VertexShaderDesc::VSI_Count; i++)
    {
        if (VertexShaderDesc::Descs[i])
        {
            StaticVShaders[i].Init(VertexShaderDesc::Descs[i], VertexShaderDesc::Descs[i]->pBinary);
        }
    }

    for (unsigned i = 0; i < FragShaderDesc::FSI_Count; i++)
    {
        if (FragShaderDesc::Descs[i])
        {
            StaticFShaders[i].Init(FragShaderDesc::Descs[i], FragShaderDesc::Descs[i]->pBinary);
        }
    }
}


bool VertexShader::Init(const VertexShaderDesc* pd, const GX2VertexShader* pvs)
{
    pDesc = pd;
    pShader = pvs;
    GX2Invalidate(GX2_INVALIDATE_CPU_SHADER, pShader->shaderPtr, pShader->shaderSize);

    memset(Uniforms, 0xff, sizeof(Uniforms));
    for (int i = 0; i < WiiU::Uniform::SU_Count; i++)
        if (pDesc->Uniforms[i].Location >= 0)
        {
            const GX2UniformVar* uvar = GX2GetVertexUniformVar(pShader, ShaderUniformNames[i]);
            if (uvar)
                Uniforms[i] = uvar->offset;
            else
            {
                char arrayname[128];
                strcpy(arrayname, ShaderUniformNames[i]);
                strcat(arrayname, "[0]");
                uvar = GX2GetVertexUniformVar(pShader, arrayname);

                if (uvar)
                    Uniforms[i] = uvar->offset;
                else
                    Uniforms[i] = 0x10000;
            }
        }

    return true;
}

bool FragShader::Init(const FragShaderDesc* pd, const GX2PixelShader* pfs)
{
    pDesc = pd;
    pShader = pfs;
    GX2Invalidate(GX2_INVALIDATE_CPU_SHADER, pShader->shaderPtr, pShader->shaderSize);

    memset(Uniforms, 0xff, sizeof(Uniforms));

    for (int i = 0; i < WiiU::Uniform::SU_Count; i++)
    {
        if (pDesc->Uniforms[i].Location >= 0)
        {
            const GX2UniformVar* uvar = GX2GetPixelUniformVar(pShader, ShaderUniformNames[i]);
            if (uvar)
            {
                Uniforms[i] = uvar->offset;
                continue;
            }

            // Try an array type variable.
            char arrayname[128];
            strcpy(arrayname, ShaderUniformNames[i]);
            strcat(arrayname, "[0]");
            uvar = GX2GetPixelUniformVar(pShader, arrayname);
            if (uvar)
            {
                Uniforms[i] = uvar->offset;
                continue;
            }

            // Try a sampler type (ensure they are in the correct location
            const GX2SamplerVar* samplerVar = GX2GetPixelSamplerVar(pShader, ShaderUniformNames[i]);
            if ( samplerVar )
            {
                SF_DEBUG_ASSERT(pDesc->Uniforms[i].Location == samplerVar->location,
                    "Sampler locations do not match between compiled shader and descriptor.");
                continue;
            }
            samplerVar = GX2GetPixelSamplerVar(pShader, arrayname);
            if ( samplerVar )
            {
                SF_DEBUG_ASSERT(pDesc->Uniforms[i].Location == samplerVar->location,
                    "Sampler locations do not match between compiled shader and descriptor.");
                continue;
            }
        }
    }
    return true;
}

class VertexBuilder
{
public:
    GX2AttribStream  Attrs[16];
    int              NumAttrs;

    void Add(int i, int attr, int ac, int offset)
    {
        GX2AttribFormat vet = GX2_ATTRIB_FORMAT_FIRST;
        switch ((attr & VET_CompType_Mask) | ac)
        {
        case VET_U8   | 1: vet = GX2_ATTRIB_FORMAT_8_UINT_TO_FLOAT; break;
        case VET_U8   | 2: vet = GX2_ATTRIB_FORMAT_8_8_UINT_TO_FLOAT; break;
        case VET_U8   | 4: vet = GX2_ATTRIB_FORMAT_8_8_8_8_UINT_TO_FLOAT; break;
        case VET_U8N  | 1: vet = GX2_ATTRIB_FORMAT_8_UNORM; break;
        case VET_U8N  | 2: vet = GX2_ATTRIB_FORMAT_8_8_UNORM; break;
        case VET_U8N  | 4: vet = GX2_ATTRIB_FORMAT_8_8_8_8_UNORM; break;
        case VET_U16  | 2: vet = GX2_ATTRIB_FORMAT_16_16_UINT_TO_FLOAT; break;
        case VET_S16  | 2: vet = GX2_ATTRIB_FORMAT_16_16_SINT_TO_FLOAT; break;
        case VET_F32  | 1: vet = GX2_ATTRIB_FORMAT_32_FLOAT; break;
        case VET_F32  | 2: vet = GX2_ATTRIB_FORMAT_32_32_FLOAT; break;
        case VET_F32  | 4: vet = GX2_ATTRIB_FORMAT_32_32_32_32_FLOAT; break;
        default: SF_ASSERT(0); break;
        }

        if ((attr & VET_Usage_Mask) == VET_Instance)
            vet = GX2_ATTRIB_FORMAT_8_UINT;

        GX2InitAttribStream(&Attrs[i], i, 0, offset, vet);
    }
    void Finish(int i)
    {
        NumAttrs = i;
    }
};

SysVertexFormat::SysVertexFormat(Render::MemoryManager* pmemmgr, const VertexFormat* pvf)
    : pMemManager(pmemmgr)
{
    VertexBuilder vb;
    BuildVertexArray(pvf, vb);

    CodeSize = GX2CalcFetchShaderSize(vb.NumAttrs);
    pShaderCode = pMemManager->Alloc(CodeSize, GX2_SHADER_ALIGNMENT, Memory_Normal);

    GX2InitFetchShader(&FetchShader, pShaderCode, vb.NumAttrs, vb.Attrs);
    GX2Invalidate(GX2_INVALIDATE_CPU_SHADER, pShaderCode, CodeSize);
}

SysVertexFormat::~SysVertexFormat()
{
    pMemManager->Free(pShaderCode, CodeSize);
}

bool ShaderInterface::SetStaticShader(ShaderDesc::ShaderType shader, const VertexFormat* pvf)
{
    VertexShaderDesc::ShaderIndex vsIndex = VertexShaderDesc::GetShaderIndex(shader);
    FragShaderDesc::ShaderIndex fsIndex = FragShaderDesc::GetShaderIndex(shader);

    CurShaders.pVS    = &StaticVShaders[vsIndex];
    CurShaders.pVDesc = CurShaders.pVS->pDesc;
    CurShaders.pFS    = &StaticFShaders[fsIndex];
    CurShaders.pFDesc = CurShaders.pFS->pDesc;
    CurShaders.pVF    = pvf->pSysFormat;
    CurShaders.pVFS   = &((const SysVertexFormat*)pvf->pSysFormat.GetPtr())->FetchShader;

    SF_ASSERT((UPInt(CurShaders.pVFS->shaderPtr) & 0xff) == 0);
    SF_ASSERT((UPInt(CurShaders.pVS->pShader->shaderPtr) & 0xff) == 0);
    SF_ASSERT((UPInt(CurShaders.pFS->pShader->shaderPtr) & 0xff) == 0);

    for (int i = 0; i < CurShaders.pVDesc->NumAttribs; i++)
    {
        UInt32 location = GX2GetVertexAttribVarLocation(CurShaders.pVS->pShader, CurShaders.pVDesc->Attributes[i].Name);
        SF_ASSERT(location == i);
    }

    GX2SetShaders(CurShaders.pVFS, CurShaders.pVS->pShader, CurShaders.pFS->pShader);
    return true;
}

void ShaderInterface::Finish(unsigned meshCount)
{
    for (int i = 0; i < Uniform::SU_Count; i++)
        if (UniformSet[i])
        {
            if (CurShaders.pFS->Uniforms[i] >= 0)
            {
                SF_ASSERT(CurShaders.pFDesc->Uniforms[i].Location >= 0);

                unsigned size;
                if (CurShaders.pFDesc->Uniforms[i].BatchSize > 0)
                    size = meshCount * CurShaders.pFDesc->Uniforms[i].BatchSize * CurShaders.pFDesc->Uniforms[i].ElementSize;
                else
                    size = CurShaders.pFDesc->Uniforms[i].Size;

                GX2SetPixelUniformReg(CurShaders.pFS->Uniforms[i], size,
                                      UniformData + CurShaders.pFDesc->Uniforms[i].ShadowOffset);
            }
            else
            {
                unsigned size;
                if (CurShaders.pVDesc->Uniforms[i].BatchSize > 0)
                    size = meshCount * CurShaders.pVDesc->Uniforms[i].BatchSize * CurShaders.pVDesc->Uniforms[i].ElementSize;
                else
                    size = CurShaders.pVDesc->Uniforms[i].Size;

                GX2SetVertexUniformReg(CurShaders.pVS->Uniforms[i], size,
                                       UniformData + CurShaders.pVDesc->Uniforms[i].ShadowOffset);
            }
        }

    memset(UniformSet, 0, Uniform::SU_Count);
}

void ShaderInterface::SetTexture(ShaderPair shaders, unsigned var, Render::Texture* ptex, ImageFillMode fm, unsigned index)
{
    const FragShader* sd = shaders.pFS;
    ptex->ApplyTexture(sd->pDesc->Uniforms[var].Location + index, fm);
}


}}}
