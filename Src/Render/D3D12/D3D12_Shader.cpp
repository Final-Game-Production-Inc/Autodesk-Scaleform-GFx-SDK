/**************************************************************************

    Filename    :   D3D12_Shader.cpp
    Content     :   D3D12 shader manager and interface.
    Created     :   2026/10/20
    Authors     :   Final Game Production Inc.

    Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                         Copyright 2026 Final Game Production Inc. All Rights Reserved.

    Use of this software is subject to the terms of the Autodesk license
    agreement provided at the time of installation or download, or which
    otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/D3D12/D3D12_Shader.h"
#include "Render/D3D12/D3D12_HAL.h"
#include "Render/D3D12/D3D12_Texture.h"
#include "Kernel/SF_Debug.h"
#include "Render/Render_DebugEnv.h"

namespace Scaleform { namespace Render { namespace D3D12 {

// Uniform layout used by all D3D12 static shaders. The CPU-side shadow
// buffer (ShaderInterfaceBase::UniformData) is laid out as the vertex
// uniform block followed by the fragment uniform block:
//   [0, SU_VertexSize)   - vertex shader uniforms
//   [SU_VertexSize, SU_VertexSize + SU_FragSize) - fragment shader uniforms
// A single ring-buffer slot per draw is used on the GPU. D3D12 constant
// buffers must be multiples of 256 bytes, so the vertex block (960 floats,
// 3840 bytes) occupies its own aligned slot followed by the fragment block:
//   [0, 3840)         - vertex constant buffer (960 floats)
//   [3840, 4096)      - fragment constant buffer (40 floats, padded to 256)
const UPInt UniformBufferSlotSize   = 256;
const UPInt UniformBufferSlotStride = 4096;
const UPInt VertexUniformSize       = Uniform::SU_VertexSize * sizeof(float);
const UPInt FragUniformSize         = Uniform::SU_FragSize   * sizeof(float);
// Vertex uniform block size, padded up to a 256-byte boundary.
const UPInt VertexUniformSlotSize   = (VertexUniformSize + UniformBufferSlotSize - 1) &
                                      ~(UniformBufferSlotSize - 1);

bool VertexShader::Init(ID3D12Device* pdevice, const VertexShaderDesc* pd)
{
    pDesc = pd;
    pDevice = pdevice;
    if (pd && pd->pBinary && pd->BinarySize)
    {
        Bytecode.pShaderBytecode = pd->pBinary;
        Bytecode.BytecodeLength  = pd->BinarySize;
    }
    return true;
}

void VertexShader::Shutdown()
{
    Bytecode.pShaderBytecode = 0;
    Bytecode.BytecodeLength  = 0;
    pDesc = 0;
    pDevice = 0;
}

bool FragShader::Init(ID3D12Device* pdevice, const FragShaderDesc* pd)
{
    pDesc = pd;
    pDevice = pdevice;
    if (pd && pd->pBinary && pd->BinarySize)
    {
        Bytecode.pShaderBytecode = pd->pBinary;
        Bytecode.BytecodeLength  = pd->BinarySize;
    }
    return true;
}

void FragShader::Shutdown()
{
    Bytecode.pShaderBytecode = 0;
    Bytecode.BytecodeLength  = 0;
    pDesc = 0;
    pDevice = 0;
    Offset = 0;
}

SysVertexFormat::SysVertexFormat(const VertexFormat* vf, const VertexShaderDesc* pvdesc)
{
    // The D3D12 input layout for a given vertex shader is precomputed by the
    // shader compiler (GFxShaderMaker) and stored in the vertex shader
    // descriptor (D3D12Attributes). The vertex format is expected to match
    // those attribute definitions; this assertion catches mismatches.
    const VertexElement* ve = vf->GetElement(VET_Pos, VET_Usage_Mask);
    SF_ASSERT(ve); SF_UNUSED(ve); // must have position.

    VertexElementCount = 0;
    if (pvdesc)
    {
        unsigned count = pvdesc->NumAttribs;
        if (count > MaxVertexElements)
            count = MaxVertexElements;
        for (unsigned i = 0; i < count; i++)
            VertexElements[i] = pvdesc->D3D12Attributes[i];
        VertexElementCount = count;
    }
}

bool ShaderInterface::SetStaticShader(ShaderDesc::ShaderType shader, const VertexFormat* pformat)
{
    CurShaders.pVFormat = pformat;
    CurShaders.pVS      = &pHal->SManager.StaticVShaders[VertexShaderDesc::GetShaderIndex(shader, pHal->SManager.ShaderModel)];
    CurShaders.pVDesc   = CurShaders.pVS->pDesc;
    CurShaders.pFS      = &pHal->SManager.StaticFShaders[FragShaderDesc::GetShaderIndex(shader, pHal->SManager.ShaderModel)];
    CurShaders.pFDesc   = CurShaders.pFS->pDesc;

    if ( pformat && !pformat->pSysFormat )
        (const_cast<VertexFormat*>(pformat))->pSysFormat = *SF_NEW SysVertexFormat(pformat, CurShaders.pVS->pDesc);

    return (bool)CurShaders;
}

void ShaderInterface::SetTexture(Shader, unsigned var, Render::Texture* ptexture, ImageFillMode fm, unsigned index)
{
    D3D12::Texture *pd3dTexture = (D3D12::Texture*)ptexture;
    SF_ASSERT(CurShaders.pFDesc->Uniforms[var].Location >= 0 );
    SF_ASSERT(CurShaders.pFDesc->Uniforms[var].Location + CurShaders.pFDesc->Uniforms[var].Size >=
        (short)(index + ptexture->TextureCount));
    pd3dTexture->ApplyTexture(CurShaders.pFDesc->Uniforms[var].Location + index, fm);
}

void ShaderInterface::Finish(unsigned meshCount)
{
    ShaderInterfaceBase::Finish(meshCount);

    HAL* hal = pHal;

    D3D12_GPU_VIRTUAL_ADDRESS vsAddress = 0;
    D3D12_GPU_VIRTUAL_ADDRESS fsAddress = 0;
    UByte* pVS = (UByte*)hal->MapUniformBuffer(&vsAddress, UniformBufferSlotStride);
    UByte* pFS = (UByte*)hal->MapUniformBuffer(&fsAddress, UniformBufferSlotStride);
    SF_ASSERT(pVS && pFS);

    memset(pVS, 0, VertexUniformSlotSize);
    memset(pFS, 0, UniformBufferSlotStride);

    for (int i = 0; i < Uniform::SU_Count; i++)
    {
        if (!UniformSet[i])
            continue;

        const UniformVar& fv = CurShaders.pFDesc->Uniforms[i];
        if (!fv.IsSampler && fv.Location >= 0)
        {
            memcpy(pFS + fv.Location * 16, UniformData + fv.ShadowOffset,
                   fv.Size * sizeof(float));
            continue;
        }

        const UniformVar& vv = CurShaders.pVDesc->Uniforms[i];
        if (!vv.IsSampler && vv.Location >= 0)
            memcpy(pVS + vv.Location * 16, UniformData + vv.ShadowOffset,
                   vv.Size * sizeof(float));
    }

    hal->SetCurrentConstantBuffers(vsAddress, fsAddress);

    // Debug probe (GFX_D3D12_UNIPROBE=1): for the shadow/blur fragment shaders,
    // compare the CPU-side fsize/scolor values with what was actually written
    // into the fragment constant buffer. A mismatch explains a runaway dynamic
    // loop in the filter shader (GPU hang) even though validation is clean.
    static const bool sUniProbe = Render::IsDebugEnvSet("GFX_D3D12_UNIPROBE");
    if (sUniProbe && CurShaders.pFDesc)
    {
        const ShaderDesc::ShaderType st = (ShaderDesc::ShaderType)CurShaders.pFDesc->Type;
        const bool shadow = (st >= ShaderDesc::ST_start_shadows && st <= ShaderDesc::ST_end_shadows);
        const bool box    = (st == ShaderDesc::ST_Box1Blur || st == ShaderDesc::ST_Box1BlurMul ||
                             st == ShaderDesc::ST_Box2Blur || st == ShaderDesc::ST_Box2BlurMul);
        if (shadow || box)
        {
            static int sLog = 0;
            if (sLog++ < 6000)
            {
                const UniformVar& fv = CurShaders.pFDesc->Uniforms[Uniform::SU_fsize];
                float got[4] = { 0, 0, 0, 0 };
                if (fv.Location >= 0)
                    memcpy(got, pFS + fv.Location * 16, sizeof(got));
                const float* cpu = UniformData + fv.ShadowOffset;
                fprintf(stderr, "[UNIPROBE] type=%d fsize loc=%d shadow=%d size=%d "
                                "cpu=%.3f,%.3f,%.3f,%.3f gpu=%.3f,%.3f,%.3f,%.3f\n",
                        (int)st, (int)fv.Location, (int)fv.ShadowOffset, (int)fv.Size,
                        cpu[0], cpu[1], cpu[2], cpu[3], got[0], got[1], got[2], got[3]);
                fflush(stderr);
            }
        }
    }

    memset(UniformSet, 0, sizeof(UniformSet));
}

void ShaderInterface::BeginScene()
{
    // No persistent D3D12 state to reset; pipeline state is resolved lazily
    // for each draw call by the HAL.
}

// *** ShaderManager
ShaderManager::ShaderManager(ProfileViews* prof) :
    StaticShaderManager(prof), pDevice(0), ShaderModel(ShaderDesc::ShaderVersion_Default)
{
    memset(StaticVShaders, 0, sizeof(StaticVShaders));
    memset(StaticFShaders, 0, sizeof(StaticFShaders));
}

ShaderManager::ShaderManager(Render::HAL* hal) :
    StaticShaderManager(&hal->GetProfiler()), pDevice(0), ShaderModel(ShaderDesc::ShaderVersion_Default)
{
    memset(StaticVShaders, 0, sizeof(StaticVShaders));
    memset(StaticFShaders, 0, sizeof(StaticFShaders));
}

void ShaderManager::MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,                                    const VertexFormat** single, const VertexFormat** batch,
                                    const VertexFormat** instanced)
{
    // The D3D12 shaders expect position data in float4 format. Promote all
    // position attributes to float so that the vertex layout matches the
    // shader's D3D12_INPUT_ELEMENT_DESC entries.
    VertexElement floatPositionElements[8];
    VertexFormat floatPositionFormat;
    floatPositionFormat.pElements = floatPositionElements;
    floatPositionFormat.pSysFormat = 0;
    unsigned offset = 0;
    unsigned i;
    for ( i = 0; sourceFormat->pElements[i].Attribute != VET_None; ++i )
    {
        floatPositionElements[i].Attribute = sourceFormat->pElements[i].Attribute;
        if ( (floatPositionElements[i].Attribute&VET_Usage_Mask) == VET_Pos)
        {
            floatPositionElements[i].Attribute &= ~VET_CompType_Mask;
            floatPositionElements[i].Attribute |= VET_F32;
            floatPositionElements[i].Offset = sourceFormat->pElements[i].Offset + offset;
            if ( (floatPositionElements[i].Attribute&VET_CompType_Mask) == VET_S16)
                offset += 4;
        }
        else
        {
            floatPositionElements[i].Offset = sourceFormat->pElements[i].Offset + offset;
        }
    }
    floatPositionElements[i].Attribute = VET_None;
    floatPositionElements[i].Offset    = 0;

    // Now let the base class actually do the mapping. Use the real per-batch
    // formats: forcing them to the single-mesh format made every DP_Batch /
    // DP_Instanced primitive declare the single-mesh element layout, so the
    // data conversion and the shader input layout disagreed and batches with a
    // different format (vector fills and strokes) came out invisible.
    const VertexFormat* localSingle    = 0;
    const VertexFormat* localBatch     = 0;
    const VertexFormat* localInstanced = 0;
    Base::MapVertexFormat(fill, &floatPositionFormat, &localSingle, &localBatch, &localInstanced,
        (HasInstancingSupport() ? MVF_HasInstancing : 0) | MVF_Align);

    if (single)
        *single = localSingle;
    if (batch)
        *batch = localBatch ? localBatch : localSingle;
    if (instanced)
        *instanced = localInstanced ? localInstanced : localSingle;

}

unsigned ShaderManager::SetupFilter(const Filter* filter, unsigned fillFlags,
                                    unsigned* passes, BlurFilterState& leBlur) const
{
    const unsigned passCount = Base::SetupFilter(filter, fillFlags, passes, leBlur);

    // The generic path is fine for single-pass filters and for non-blur kinds.
    if (passCount <= 1 || filter->GetFilterType() > Filter_Blur_End)
        return passCount;

    // Re-enable the intermediate Box passes for debugging only.
    static const bool sMultiPass = Render::IsDebugEnvSet("GFX_D3D12_MULTIPASS_BLUR");
    if (sMultiPass)
        return passCount;

    // Collapse to the final pass (the shadow/glow shader). It applies the same
    // 2D blur in one loop, so nothing renders into a temporary target and then
    // samples that same target back on the next pass - the ping-pong that hung
    // the GPU on the feedback banner rebuild.
    passes[0] = passes[passCount - 1];
    return 1;
}

bool ShaderManager::Initialize(HAL* phal, unsigned configFlags)
{
    SF_UNUSED(configFlags);
    pDevice = phal->GetDevice();
    ShaderModel = ShaderDesc::ShaderVersion_D3D12;

    for (unsigned i = 0; i < VertexShaderDesc::VSI_Count; i++)
    {
        const VertexShaderDesc* desc = VertexShaderDesc::Descs[i];
        if ( desc && desc->Version == ShaderModel && desc->pBinary)
        {
            if (!StaticVShaders[i].Init(pDevice, desc))
                return false;
        }
    }

    for (unsigned i = 0; i < FragShaderDesc::FSI_Count; i++)
    {
        const FragShaderDesc* desc = FragShaderDesc::Descs[i];
        if (desc && desc->Version == ShaderModel && desc->pBinary)
        {
            if ( !StaticFShaders[i].Init(pDevice, desc) )
                return false;
        }
    }

    return true;
}

void ShaderManager::OnHALEvent(HALNotifyType type)
{
    if (type == HAL_PrepareForReset || type == HAL_Shutdown)
        Reset();
}

void ShaderManager::BeginScene()
{
}

void ShaderManager::EndScene()
{
}

void ShaderManager::Reset()
{
    for (unsigned i = 0; i < VertexShaderDesc::VSI_Count; i++)
    {
        const VertexShaderDesc* desc = VertexShaderDesc::Descs[i];
        if ( desc && desc->pBinary)
            StaticVShaders[i].Shutdown();
    }

    for (unsigned i = 0; i < FragShaderDesc::FSI_Count; i++)
    {
        const FragShaderDesc* desc = FragShaderDesc::Descs[i];
        if (desc && desc->pBinary)
            StaticFShaders[i].Shutdown();
    }

    pDevice = 0;
    VFormats.Clear();
}

}}} // Scaleform::Render::D3D12
